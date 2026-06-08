---
title: HTTP半包未读全与连接释放机制
aliases:
  - TinyWebServer HTTP没读全会不会阻塞
  - HTTP半包和NO_REQUEST处理
tags:
  - tinywebserver
  - http
  - epoll
  - nonblocking
  - timer
  - interview
created: 2026-05-21
---

# 20-HTTP半包未读全与连接释放机制

关联笔记：
- [[TinyWebServer-面试拆解笔记]]
- [[TinyWebServer-拆解/04-http_conn与HTTP状态机]]
- [[TinyWebServer-拆解/06-定时器与连接管理]]
- [[TinyWebServer-拆解/14-HTTP状态机逐请求模拟]]
- [[TinyWebServer-拆解/16-客户端读请求处理全流程模拟]]

## 这篇笔记讲什么

这篇专门回答一个面试里很容易被追问的问题：

> 如果 HTTP 请求没有一次性读完整，会不会导致服务器或者工作线程一直阻塞？

进一步还要回答：

1. 为什么 HTTP 会出现“没读全”
2. `recv()` 会不会一直等完整请求
3. 工作线程会不会一直卡在这个任务上
4. 半包请求最后什么时候继续处理
5. 如果客户端一直不补齐数据，这个连接什么时候释放

---

## 先说结论

不会因为 HTTP 没读全就一直阻塞。

这个项目里，客户端连接 fd 被加入 `epoll` 时会设置成**非阻塞**。所以 `recv()` 只会读取当前内核缓冲区里已经到达的数据。如果数据暂时不够组成完整 HTTP 请求，状态机会返回 `NO_REQUEST`，然后 `process()` 会重新监听 `EPOLLIN` 并直接返回。

所以准确说是：

> HTTP 半包不会占住工作线程一直等，但会占住一个连接资源。后面要么客户端继续发数据，要么客户端断开，要么定时器超时回收。

这句话面试时非常关键。

---

## 为什么 HTTP 会没读全

因为 HTTP 通常跑在 TCP 上，而 TCP 是**字节流协议**。

TCP 不保证：

- 一次 `recv()` 就收到完整 HTTP 请求
- 一次 `recv()` 只收到一个 HTTP 请求
- 请求行、请求头、请求体刚好按边界到达

所以可能出现：

1. 只收到半个请求行
2. 请求头还没收到空行
3. POST 请求头收到了，但请求体还没收全
4. `\r` 到了，但后面的 `\n` 还没到

这就是为什么项目里要用：

- 从状态机：`parse_line()`
- 主状态机：`process_read()`

---

## 第 1 步：连接 fd 被设置成非阻塞

连接加入 epoll 的函数是：

```cpp
void addfd(int epollfd, int fd, bool one_shot, int TRIGMode)
{
    ...
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);
}
```

而 `setnonblocking(fd)` 做的是：

```cpp
int old_option = fcntl(fd, F_GETFL);
int new_option = old_option | O_NONBLOCK;
fcntl(fd, F_SETFL, new_option);
```

也就是说，只要这个 fd 被加入 epoll，它就变成非阻塞 fd。

非阻塞的含义是：

> 如果当前没有更多数据可读，`recv()` 不会一直睡在那里等，而是返回错误，并把 `errno` 设成 `EAGAIN` 或 `EWOULDBLOCK`。

---

## 第 2 步：`read_once()` 只负责读当前已经到达的数据

读数据入口是：

```cpp
bool http_conn::read_once()
```

它不是“读到完整 HTTP 请求为止”，而是：

> 把当前 socket 里已经到达的数据读到 `m_read_buf` 里。

### LT 模式

LT 模式下只读一次：

```cpp
bytes_read = recv(m_sockfd,
                  m_read_buf + m_read_idx,
                  READ_BUFFER_SIZE - m_read_idx,
                  0);
m_read_idx += bytes_read;
```

如果这一次只读到半个 HTTP 请求，也没关系。

新数据会追加到：

```cpp
m_read_buf + m_read_idx
```

不会覆盖上一次没解析完的数据。

### ET 模式

ET 模式下会循环读：

```cpp
while (true)
{
    bytes_read = recv(...);
    if (bytes_read == -1)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            break;
        return false;
    }
    ...
}
```

ET 模式必须一直读到 `EAGAIN`，原因是：

> ET 是边沿触发。如果这次没有把内核缓冲区读空，后面可能不会再触发新的读事件。

但是注意，即使 ET 模式循环读，它也只是读到“当前没有更多数据”为止，不是读到“HTTP 请求完整”为止。

---

## 第 3 步：如果一行还没完整，`parse_line()` 返回 `LINE_OPEN`

HTTP 请求行和请求头都以 `\r\n` 结束。

`parse_line()` 会从 `m_checked_idx` 开始往后扫，判断当前缓冲区里有没有完整的一行。

如果遇到这种情况：

```text
GET /index.html HTTP/1.1\r
```

也就是只有 `\r`，还没有 `\n`，代码会返回：

```cpp
return LINE_OPEN;
```

如果扫到 `m_read_idx` 位置都没有找到完整的 `\r\n`，也返回：

```cpp
return LINE_OPEN;
```

`LINE_OPEN` 的含义就是：

> 当前缓冲区里的数据还不够切出一整行，需要等更多数据到来。

---

## 第 4 步：请求没完整时，`process_read()` 返回 `NO_REQUEST`

HTTP 解析总入口是：

```cpp
HTTP_CODE http_conn::process_read()
```

核心循环是：

```cpp
while ((m_check_state == CHECK_STATE_CONTENT && line_status == LINE_OK)
       || ((line_status = parse_line()) == LINE_OK))
{
    ...
}

return NO_REQUEST;
```

也就是说，如果：

- 请求行没收完整
- 请求头没收完整
- POST 请求体没收完整

状态机都会停下来，最后返回：

```cpp
NO_REQUEST
```

`NO_REQUEST` 不是错误。

它表示：

> 当前请求还没完整，不能进入 `do_request()`，需要继续等 socket 上的新数据。

---

## 第 5 步：`process()` 收到 `NO_REQUEST` 后会直接返回

单连接处理入口是：

```cpp
void http_conn::process()
{
    HTTP_CODE read_ret = process_read();
    if (read_ret == NO_REQUEST)
    {
        modfd(m_epollfd, m_sockfd, EPOLLIN, m_TRIGMode);
        return;
    }

    bool write_ret = process_write(read_ret);
    ...
    modfd(m_epollfd, m_sockfd, EPOLLOUT, m_TRIGMode);
}
```

这里非常关键。

如果 HTTP 没读全，代码不会在 `process()` 里死循环等剩余数据，而是：

1. 重新注册 `EPOLLIN`
2. 解除 `EPOLLONESHOT`
3. 直接 `return`

所以工作线程处理完这一轮任务后，会回到线程池循环里，继续等待其他任务。

---

## 工作线程会不会一直待在这个任务中

不会。

在 Reactor 模式下，工作线程处理读任务时大致是：

```cpp
if (request->read_once())
{
    request->improv = 1;
    connectionRAII mysqlcon(&request->mysql, m_connPool);
    request->process();
}
```

如果 `process()` 发现请求没完整，它会重新监听 `EPOLLIN` 然后返回。

于是当前工作线程就完成了这个任务，不会继续占着这个连接等。

在 Proactor 模式下，主线程先 `read_once()`，再把任务交给工作线程：

```cpp
if (users[sockfd].read_once())
{
    m_pool->append_p(users + sockfd);
}
```

工作线程执行 `request->process()`，遇到 `NO_REQUEST` 同样会返回。

所以两种模式下结论一样：

> 半包不会一直占用工作线程。

---

## 那这个半包数据保存在哪里

保存在当前连接对象 `http_conn` 的读缓冲区里：

```cpp
char m_read_buf[READ_BUFFER_SIZE];
int m_read_idx;
int m_checked_idx;
int m_start_line;
```

几个变量的含义是：

| 变量 | 含义 |
|---|---|
| `m_read_buf` | 已经读到用户态的 HTTP 原始数据 |
| `m_read_idx` | 当前读缓冲区有效数据的尾部位置 |
| `m_checked_idx` | 从状态机已经检查到哪里 |
| `m_start_line` | 当前待解析行的起始位置 |
| `m_check_state` | 主状态机当前处于请求行、请求头还是请求体 |

下次这个 fd 再有 `EPOLLIN` 时，新的数据会继续追加到 `m_read_buf + m_read_idx` 后面，然后状态机接着上一次的位置继续解析。

---

## 如果客户端后面继续发送数据

流程是：

```text
客户端继续发送剩余 HTTP 数据
-> epoll_wait 再次发现 EPOLLIN
-> dealwithread(sockfd)
-> read_once() 把新数据追加到 m_read_buf
-> process_read() 接着上次的位置继续解析
```

如果这一次请求终于完整了，`process_read()` 就会返回：

- `GET_REQUEST`
- `FILE_REQUEST`
- `BAD_REQUEST`
- 其他 HTTP_CODE

然后 `process()` 才会进入：

```cpp
process_write(read_ret);
modfd(..., EPOLLOUT, ...);
```

也就是开始组织响应，并切换到写事件。

---

## 如果客户端一直不发完整请求

如果客户端发了一半 HTTP 请求后就不动了，比如只发：

```text
POST /2CGISQL.cgi HTTP/1.1\r\n
Host: 127.0.0.1:9006\r\n
Content-Length: 100\r\n
\r\n
user=ali
```

但后面的 body 一直不发，那么：

- 工作线程不会被占住
- 这个 fd 仍然留在 epoll 里
- `http_conn` 对象和读缓冲区还占着
- 定时器还绑定着这个连接

最后释放连接要靠定时器。

---

## 定时器什么时候释放这个半包连接

这个项目里每个连接都会绑定一个定时器。连接有活动时会调整定时器过期时间；长时间没有活动，就会被认为是空闲连接。

整体链路是：

```text
alarm 周期性触发 SIGALRM
-> 信号处理函数把信号写入管道
-> epoll_wait 发现管道可读
-> 主循环处理 SIGALRM
-> timer_handler()
-> sort_timer_lst::tick()
-> 找到已经超时的连接
-> cb_func()
-> epoll 删除 fd 并 close
```

所以如果客户端一直不补齐 HTTP 请求，它不会让工作线程一直阻塞，但会占着一个连接资源，直到定时器判定超时并关闭。

---

## 面试官追问：那这是不是 Slowloris 攻击风险

可以回答：

是的，从工程角度看，这类“慢慢发、不发完整请求”的行为就是 Slowloris 类攻击的思路。它不一定占用工作线程，但会占用连接 fd、读缓冲区、定时器节点和 epoll 资源。

这个项目通过定时器能回收长时间不活跃连接，但如果要更稳，还可以继续优化：

1. 设置更短的请求头读取超时
2. 限制单 IP 最大连接数
3. 限制请求头最大长度
4. 对 header 长时间不完整的连接直接关闭
5. 前面加 Nginx 或负载均衡做连接层防护

---

## 面试时可以怎么回答

> 不会一直阻塞。因为项目里客户端连接 fd 加入 epoll 后会被设置成非阻塞，`read_once()` 只是把当前已经到达的数据读到 `m_read_buf`，不会为了等完整 HTTP 请求而卡住。如果请求行、请求头或者 POST body 还没收全，状态机会返回 `NO_REQUEST`，`process()` 会重新注册 `EPOLLIN` 并返回，工作线程就结束当前任务，回到线程池继续处理其他请求。
>
> 半包数据会保留在这个连接自己的读缓冲区里，下次这个 fd 再触发读事件时继续追加和解析。如果客户端一直不补齐请求，这个连接不会占住工作线程，但会占用 fd、缓冲区和定时器资源，最后靠定时器超时回收。工程上如果要防慢请求攻击，还可以加请求头超时、单 IP 连接数限制和请求大小限制。

