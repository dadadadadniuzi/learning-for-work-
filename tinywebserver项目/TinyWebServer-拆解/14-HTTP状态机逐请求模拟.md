---
title: HTTP状态机逐请求模拟
aliases:
  - TinyWebServer HTTP 状态机逐步模拟
  - http_conn 逐请求走读
tags:
  - tinywebserver
  - http
  - state-machine
  - cpp
created: 2026-04-30
---
   
# 14-HTTP状态机逐请求模拟

关联笔记：
- [[TinyWebServer-面试拆解笔记]]
- [[TinyWebServer-拆解/04-http_conn与HTTP状态机]]
- [[TinyWebServer-拆解/10-HTTP请求报文解析与GETPOST流程]]

## 这篇笔记解决什么问题

这篇不是讲抽象概念，而是直接模拟：

1. 一个连接刚 `init()` 完时，`http_conn` 里关键变量是什么值
2. `read_once()` 把请求读进来后，`m_read_idx / m_checked_idx / m_start_line` 怎么变
3. `process()` 是怎么一步一步把请求交给状态机的
4. `parse_line()`、`parse_request_line()`、`parse_headers()`、`parse_content()` 每一步到底在干什么
5. 你给出的 `GET` 和 `POST` 请求，在这个项目里分别会走出什么结果

---

## 先记住 3 个最关键的下标

### `m_read_idx`

表示：读缓冲区里，现在一共已经有多少个有效字节。

可以把它理解为：

`m_read_buf[0 ... m_read_idx-1]` 这段数据是当前已经从 socket 读进来的。

### `m_checked_idx`

表示：从状态机已经检查到哪里了。

它主要被 `parse_line()` 推进。

可以把它理解为：

`m_read_buf[0 ... m_checked_idx-1]` 这段，状态机已经扫描过了。

### `m_start_line`

表示：当前要解析的这一行，在 `m_read_buf` 里的起始下标。

每次 `get_line()` 实际返回的就是：

```cpp
m_read_buf + m_start_line
```

也就是“当前这一行的开头地址”。

---

## 一个请求真正开始被状态机处理，是在什么时候

很多人会误以为 `webserver.cpp` 会直接调 `parse_request_line()` 或 `process_read()`，其实不是。

真实调用链是：

1. `epoll_wait` 发现某个连接 `EPOLLIN`
2. `WebServer::dealwithread(sockfd)` 开始处理这个读事件
3. 根据 Reactor / Proactor 模式不同，先读数据，或者把读任务交给线程池
4. 最终一定会调用到 `http_conn::process()`
5. `process()` 里面第一句就是：

```cpp
HTTP_CODE read_ret = process_read();
```

所以：

真正启动 HTTP 状态机的是 `http_conn::process()`。

---

## 时间线：一个连接从创建到进入状态机

### 第 0 步：新连接建立

调用：

```cpp
users[connfd].init(connfd, client_address, ...);
```

这个 `init()` 最后会调用无参版本的 `init()`，把解析状态全部清空。

### 第 1 步：`init()` 刚结束时，关键变量的值

在 [http_conn.cpp](G:\计算机学习\tinywebserver项目\TinyWebServer-master\http\http_conn.cpp) 对应逻辑里，关键状态如下：

| 变量 | 初值 | 含义 |
|---|---:|---|
| `m_check_state` | `CHECK_STATE_REQUESTLINE` | 下一步先解析请求行 |
| `m_linger` | `false` | 默认不是长连接 |
| `m_method` | `GET` | 只是初始默认值，后面会被真正请求覆盖 |
| `m_url` | `0` | 还没有解析出 URL |
| `m_version` | `0` | 还没有解析出 HTTP 版本 |
| `m_content_length` | `0` | 还不知道 body 长度 |
| `m_host` | `0` | 还没有解析出 Host |
| `m_start_line` | `0` | 当前行起始位置先从缓冲区开头算 |
| `m_checked_idx` | `0` | 还没有扫描任何字节 |
| `m_read_idx` | `0` | 还没有从 socket 读入任何字节 |
| `m_write_idx` | `0` | 还没有组织响应 |
| `cgi` | `0` | 默认不是 POST 表单处理分支 |
| `bytes_to_send` | `0` | 还没有待发送数据 |
| `bytes_have_send` | `0` | 还没有发送任何响应 |

这时候要特别记住：

- `m_start_line = 0`
- `m_checked_idx = 0`
- `m_read_idx = 0`

也就是说：

缓冲区里什么都没有，状态机也什么都还没开始看。

---

## 请求一：GET 图片请求的完整模拟

### 原始请求

```http
GET /562f25980001b1b106000338.jpg HTTP/1.1
Host:img.mukewang.com
User-Agent:Mozilla/5.0 (Windows NT 10.0; WOW64)
AppleWebKit/537.36 (KHTML, like Gecko) Chrome/51.0.2704.106 Safari/537.36
Accept:image/webp,image/*,*/*;q=0.8
Referer:http://www.imooc.com/
Accept-Encoding:gzip, deflate, sdch
Accept-Language:zh-CN,zh;q=0.8

```

说明：

- 最后一行空行很重要
- GET 请求这里没有请求体
- 按你给的文本，整个请求大约是 `330` 字节

### 第 2 步：`read_once()` 之后

假设这次网络很好，这 330 字节一次性全部读进来了。

那么此时：

| 变量 | 值 |
|---|---:|
| `m_read_idx` | `330` |
| `m_checked_idx` | `0` |
| `m_start_line` | `0` |

含义：

- 缓冲区里已经有完整请求
- 但状态机还没开始扫描
- 当前行默认还是从第 0 个字符开始

### 第 3 步：进入 `process()`

执行：

```cpp
HTTP_CODE read_ret = process_read();
```

这句话就是正式启动 HTTP 状态机。

---

## GET 请求在 `process_read()` 里的流转

### 第 4 步：第一次 `while` 循环

此时：

- `m_check_state = CHECK_STATE_REQUESTLINE`
- `line_status = LINE_OK`

由于当前不是 `CHECK_STATE_CONTENT`，所以 `while` 会先执行：

```cpp
line_status = parse_line();
```

### 第 5 步：第一次 `parse_line()`

它从：

```cpp
for (; m_checked_idx < m_read_idx; ++m_checked_idx)
```

开始扫描。

初始时：

- `m_checked_idx = 0`
- `m_read_idx = 330`

它会一直扫到第一行结尾的 `\r\n`。

第一行是：

```http
GET /562f25980001b1b106000338.jpg HTTP/1.1
```

这行长度按字节算是：

- 文本 42 字节
- `\r\n` 2 字节
- 合计 44 字节

所以第一次 `parse_line()` 结束后：

| 变量 | 值 |
|---|---:|
| `m_checked_idx` | `44` |
| `m_start_line` | 还没更新，暂时还是 `0` |
| `line_status` | `LINE_OK` |

同时它会把这一行结尾的 `\r\n` 原地改成 `\0\0`。

这一步非常关键。

因为这样 `m_read_buf + 0` 开始就变成了一个完整 C 字符串：

```cpp
"GET /562f25980001b1b106000338.jpg HTTP/1.1"
```

### 第 6 步：`get_line()` 取出当前行

接着：

```cpp
text = get_line();
m_start_line = m_checked_idx;
```

此时：

- `text = m_read_buf + 0`
- `m_start_line = 44`

注意这里的含义：

- `text` 指向刚刚那一行请求行
- `m_start_line = 44` 表示下一轮如果再 `get_line()`，默认就从第二行开始

### 第 7 步：进入 `parse_request_line(text)`

现在传进去的是：

```cpp
text = "GET /562f25980001b1b106000338.jpg HTTP/1.1"
```

它做的事情是：

1. 找到 `GET` 和 URL 之间的空格
2. 原地切开字符串
3. 识别方法是 `GET`
4. 找到 URL 和版本号之间的空格
5. 再原地切开
6. 校验版本号是 `HTTP/1.1`
7. 把状态机切到 `CHECK_STATE_HEADER`

结束后关键变量变成：

| 变量 | 值 |
|---|---|
| `m_method` | `GET` |
| `m_url` | `"/562f25980001b1b106000338.jpg"` |
| `m_version` | `"HTTP/1.1"` |
| `m_check_state` | `CHECK_STATE_HEADER` |

返回值是：

```cpp
NO_REQUEST
```

为什么不是 `GET_REQUEST`？

因为这时候只是“请求行解析完了”，请求头还没处理，所以整个请求还不能算完成。

---

## GET 请求继续解析请求头

### 第 8 步：第二次 `while` 循环

现在：

- `m_check_state = CHECK_STATE_HEADER`
- `m_start_line = 44`
- `m_checked_idx = 44`

`while` 再次调用 `parse_line()`。

它会从第 44 个字节开始扫，也就是第二行：

```http
Host:img.mukewang.com
```

这一行总长度是 23 字节。

所以这次 `parse_line()` 结束后：

| 变量 | 值 |
|---|---:|
| `m_checked_idx` | `67` |
| `line_status` | `LINE_OK` |

然后：

```cpp
text = get_line();      // 指向 Host 行
m_start_line = 67;
ret = parse_headers(text);
```

这时：

```cpp
m_host = "img.mukewang.com"
```

但返回值仍然是：

```cpp
NO_REQUEST
```

因为只是处理完了一行请求头。

### 第 9 步：后面的每个请求头都重复这个过程

后面这些行都会被一行一行切出来：

- `User-Agent:...`
- `Accept:...`
- `Referer:...`
- `Accept-Encoding:...`
- `Accept-Language:...`

这个项目只真正处理：

- `Connection`
- `Content-length`
- `Host`

其他头字段基本都是：

```cpp
LOG_INFO("oop!unknow header: %s", text);
```

也就是：

它们会被看见，但不会参与业务逻辑。

---

## GET 请求遇到空行时发生什么

### 第 10 步：空行被 `parse_line()` 切出来

请求头结束后会有一个空行，也就是一个独立的 `\r\n`。

`parse_line()` 处理完后，这一行会变成空字符串：

```cpp
text[0] == '\0'
```

然后进入：

```cpp
parse_headers(text)
```

在 `parse_headers()` 里：

```cpp
if (text[0] == '\0')
{
    if (m_content_length != 0)
    {
        m_check_state = CHECK_STATE_CONTENT;
        return NO_REQUEST;
    }
    return GET_REQUEST;
}
```

因为 GET 请求没有 `Content-Length`，所以：

- `m_content_length = 0`
- 直接返回 `GET_REQUEST`

这表示：

整个 HTTP 请求到这里已经完整了。

### 第 11 步：进入 `do_request()`

`process_read()` 收到 `GET_REQUEST` 后会立刻：

```cpp
return do_request();
```

然后开始：

1. 拼接文件真实路径
2. `stat()` 检查文件是否存在
3. `open()`
4. `mmap()`
5. 返回 `FILE_REQUEST`

到这里，HTTP 状态机这一轮的“读请求并解析”阶段就结束了。

---

## 这个 GET 请求里三个下标的关键变化

### 刚 `init()` 完

| 变量 | 值 |
|---|---:|
| `m_start_line` | `0` |
| `m_checked_idx` | `0` |
| `m_read_idx` | `0` |

### `read_once()` 把整个请求读完后

| 变量 | 值 |
|---|---:|
| `m_start_line` | `0` |
| `m_checked_idx` | `0` |
| `m_read_idx` | `330` |

### 第一行请求行被 `parse_line()` 切出来后

| 变量 | 值 |
|---|---:|
| `m_start_line` | `0` |
| `m_checked_idx` | `44` |
| `m_read_idx` | `330` |

### 进入下一轮前更新 `m_start_line`

| 变量 | 值 |
|---|---:|
| `m_start_line` | `44` |
| `m_checked_idx` | `44` |
| `m_read_idx` | `330` |

### 所有头都扫完、遇到空行后

大体上：

- `m_checked_idx` 会推进到整个请求末尾附近
- `m_start_line` 会指向空行后的下一段位置
- `m_read_idx` 仍然是 `330`

这里最重要的结论是：

- `m_read_idx` 只在“读 socket”时变化
- `m_checked_idx` 只在“扫描缓冲区”时变化
- `m_start_line` 只在“切换到下一行”时变化

---

## 请求二：POST 请求的模拟

### 你给的原始请求

```http
POST / HTTP1.1
Host:www.wrox.com
User-Agent:Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; SV1; .NET CLR 2.0.50727; .NET CLR 3.0.04506.648; .NET CLR 3.5.21022)
Content-Type:application/x-www-form-urlencoded
Content-Length:40
Connection: Keep-Alive

name=Professional%20Ajax&publisher=Wiley
```

整个请求大约是 `300` 字节。

请求体长度正好是：

```text
40
```

---

## 先说结论：按你给的原文，这个 POST 会被判成坏请求

原因在第一行：

```http
POST / HTTP1.1
```

这里的版本号写成了：

```text
HTTP1.1
```

但源码只接受：

```text
HTTP/1.1
```

也就是必须有那个 `/`。

所以在 `parse_request_line()` 里执行到：

```cpp
if (strcasecmp(m_version, "HTTP/1.1") != 0)
    return BAD_REQUEST;
```

时，会直接返回：

```cpp
BAD_REQUEST
```

这意味着：

- 后面的 `Host`
- `Content-Length`
- `Connection`
- 请求体 `name=Professional...`

统统都不会进入正常解析流程。

---

## 按你给的原文，POST 请求的变量流转

### 第 1 步：`init()` 后

和 GET 完全一样：

| 变量 | 值 |
|---|---:|
| `m_start_line` | `0` |
| `m_checked_idx` | `0` |
| `m_read_idx` | `0` |
| `m_check_state` | `CHECK_STATE_REQUESTLINE` |

### 第 2 步：`read_once()` 读完整个 POST

假设一次性读完：

| 变量 | 值 |
|---|---:|
| `m_start_line` | `0` |
| `m_checked_idx` | `0` |
| `m_read_idx` | `300` |

### 第 3 步：第一次 `parse_line()`

第一行：

```http
POST / HTTP1.1
```

这一行长度是：

- 文本 14 字节
- `\r\n` 2 字节
- 合计 16 字节

所以第一次切行后：

| 变量 | 值 |
|---|---:|
| `m_checked_idx` | `16` |
| `m_start_line` | 还没更新，暂时 `0` |
| `m_read_idx` | `300` |

### 第 4 步：进入 `parse_request_line(text)`

它会解析出：

- `method = POST`
- `m_url = "/"`
- `m_version = "HTTP1.1"`

这时执行到版本校验：

```cpp
if (strcasecmp(m_version, "HTTP/1.1") != 0)
    return BAD_REQUEST;
```

因为：

- 左边是 `HTTP1.1`
- 右边是 `HTTP/1.1`

不相等，所以直接：

```cpp
return BAD_REQUEST;
```

### 第 5 步：`process_read()` 直接结束

于是：

```cpp
if (ret == BAD_REQUEST)
    return BAD_REQUEST;
```

状态机就停了。

后续不会进入：

- `CHECK_STATE_HEADER`
- `CHECK_STATE_CONTENT`
- `parse_headers()`
- `parse_content()`

---

## 如果你想看一个“正常 POST”怎么走

只需要把请求行改成：

```http
POST / HTTP/1.1
```

下面就是正常情况。

---

## 正常 POST 请求的完整状态机过程

### 第 1 步：请求行解析成功

改正后第一行是：

```http
POST / HTTP/1.1
```

这时 `parse_request_line()` 结束后：

| 变量 | 值 |
|---|---|
| `m_method` | `POST` |
| `cgi` | `1` |
| `m_url` | `"/"` |
| `m_version` | `"HTTP/1.1"` |
| `m_check_state` | `CHECK_STATE_HEADER` |

这里 `cgi = 1` 非常重要。

因为后面 `do_request()` 里会根据它判断是否进入登录/注册这种 POST 表单逻辑。

### 第 2 步：逐行解析请求头

接下来状态机会一行一行处理：

#### `Host:www.wrox.com`

处理后：

```cpp
m_host = "www.wrox.com"
```

#### `Content-Type:application/x-www-form-urlencoded`

这个项目基本不处理 `Content-Type`，只会当成未知头打日志。

#### `Content-Length:40`

处理后：

```cpp
m_content_length = 40
```

这一步决定：

后面必须再收 `40` 个字节 body，状态机才算完整。

#### `Connection: Keep-Alive`

处理后：

```cpp
m_linger = true
```

这表示响应结束后，连接可以不马上关掉。

### 第 3 步：遇到空行

请求头结束时，`parse_headers()` 收到空串。

这时因为：

```cpp
m_content_length == 40
```

所以不会直接返回 `GET_REQUEST`，而是：

```cpp
m_check_state = CHECK_STATE_CONTENT;
return NO_REQUEST;
```

注意这里特别重要：

这意味着状态机说的是：

“请求头处理完了，但整个请求还没完整，因为后面还有 40 字节 body 要处理。”

### 第 4 步：进入 `CHECK_STATE_CONTENT`

这时 `process_read()` 的 `while` 条件会走左半边：

```cpp
m_check_state == CHECK_STATE_CONTENT && line_status == LINE_OK
```

也就是说：

进入请求体阶段后，不再靠 `parse_line()` 一行一行切。

因为 body 不是按行语义解析的，它只按长度解析。

### 第 5 步：`parse_content(text)`

此时：

- `text` 指向 body 起始位置
- `m_content_length = 40`
- body 内容是：

```text
name=Professional%20Ajax&publisher=Wiley
```

`parse_content()` 核心判断：

```cpp
if (m_read_idx >= (m_content_length + m_checked_idx))
```

含义是：

当前已经收到的总字节数，是否已经足够覆盖：

`请求体起始位置 + 40 字节`

如果够了，就说明 body 收完整了。

然后：

```cpp
text[m_content_length] = '\0';
m_string = text;
return GET_REQUEST;
```

处理后：

| 变量 | 值 |
|---|---|
| `m_string` | `"name=Professional%20Ajax&publisher=Wiley"` |
| 返回值 | `GET_REQUEST` |

注意：

虽然这里是 POST，请求体解析成功后源码仍然返回 `GET_REQUEST`。

你要把它理解成：

“请求已经完整了”

而不是“它真的是 GET 方法”。

### 第 6 步：进入 `do_request()`

`process_read()` 收到 `GET_REQUEST` 后：

```cpp
return do_request();
```

这时 `do_request()` 里就可以使用：

- `m_method = POST`
- `cgi = 1`
- `m_url`
- `m_string`

去做后续表单处理。

---

## POST 请求里三个下标怎么理解

对 POST 来说，最容易混的是：

- 为什么前半段按行切
- 后半段却不按行切

答案就是：

### 请求行和请求头

都是“按行语义”的。

所以：

- 靠 `parse_line()` 找 `\r\n`
- 靠 `m_checked_idx` 一路往后推进

### 请求体

不是按行语义，而是按 `Content-Length` 语义。

所以进入 `CHECK_STATE_CONTENT` 后：

- 不再要求 `parse_line()` 切出一行
- 只要字节数达到 `Content-Length`，就算 body 完整

这正是这个状态机设计最关键的地方。

---

## 你面试时可以怎么概括这条流程

可以直接这样说：

> `http_conn` 在连接初始化时会把 `m_start_line`、`m_checked_idx`、`m_read_idx` 都清零，并把主状态机设为 `CHECK_STATE_REQUESTLINE`。  
> `read_once()` 把请求字节流读入 `m_read_buf` 后，`process()` 调用 `process_read()` 正式启动 HTTP 状态机。  
> 请求行和请求头阶段靠 `parse_line()` 以 `\r\n` 为边界切行，再分别交给 `parse_request_line()` 和 `parse_headers()`。  
> 如果头部里发现了 `Content-Length`，空行之后主状态机会切到 `CHECK_STATE_CONTENT`，此时不再按行解析，而是按长度判断请求体是否收完整。  
> 收完整后再进入 `do_request()` 做资源定位或 POST 表单业务处理。  

---

## 最后记住 4 个最重要的结论

1. `m_read_idx` 只表示“已经读进来了多少字节”。
2. `m_checked_idx` 只表示“状态机已经扫描到哪里了”。
3. `m_start_line` 只表示“当前这一行从哪里开始”。
4. GET 通常在空行处结束，POST 通常在 `Content-Length` 满足时结束。

---

## 如果 HTTP 请求不完整，它是怎么等待完整协议继续传进来的

这部分非常重要，因为面试官经常会追问：

- 如果一次只收到半个请求头怎么办
- 如果请求头和请求体不是一次到齐怎么办
- 如果 TCP 粘包，一次来了多个请求怎么办

这套代码能处理这些情况，核心不是“等一会再说”这么简单，而是：

1. 已经收到的数据先保存在 `m_read_buf`
2. 用 `m_read_idx` 记住当前一共收到了多少字节
3. 用 `m_checked_idx` 记住已经检查到哪里了
4. 如果发现还不完整，就返回 `NO_REQUEST`（请求行和请求头用parse_line()来检测是否完整、请求数据部分用`m_read_idx`是否大于等于m_checked_idx+content_lenght来检测）
5. 主线程重新监听 `EPOLLIN`
6. 下次网络数据再到时，从上次断点继续往后读、继续往后检查

也就是说：

这不是“丢弃不完整请求重新来”，而是“把半包先留在缓冲区里，等后续字节补齐后继续解析”。

---

## 先记住一句最关键的话

### `NO_REQUEST` 的真正含义

不是：

> 这个请求没用，丢掉。

而是：

> 这个请求暂时还不完整，先保留已收到的数据，等下一次 `EPOLLIN` 再继续补。

这就是整个“不完整 HTTP 请求处理”的核心思想。

---

## 不完整请求时，谁在“等”

严格来说，不是 `http_conn` 在阻塞等待。

真正的机制是：

1. `process_read()` 发现当前缓冲区还不够组成完整请求
2. 返回 `NO_REQUEST`
3. `http_conn::process()` 收到 `NO_REQUEST`
4. 调用：

```cpp
modfd(m_epollfd, m_sockfd, EPOLLIN, m_TRIGMode);
```

5. 这表示：

> 继续把这个连接挂回 epoll，等它下次再有可读数据时重新通知我

所以“等待”不是 while 死等，也不是 sleep。

而是：

把连接重新交还给 `epoll` 事件循环，等内核告诉它“又有新数据来了”。

---

## 情况一：一次只收到半个请求头

### 例子

第一次只收到：

```http
GET /index.html HTTP/1.1\r\n
Host:img.mu
```

注意：

- 第一行请求行是完整的
- 第二行 `Host` 只有一半
- 后面没有 `\r\n`

### 第 1 步：`read_once()` 之后

此时：

- `m_read_buf` 里有半包数据
- `m_read_idx > 0`
- `m_checked_idx = 0`
- `m_start_line = 0`

### 第 2 步：`process_read()` 先处理请求行

第一次 `parse_line()` 能成功切出：

```http
GET /index.html HTTP/1.1
```

所以：

- 请求行能正常进入 `parse_request_line()`
- `m_check_state` 被切到 `CHECK_STATE_HEADER`

### 第 3 步：继续处理第二行请求头

第二次 `parse_line()` 开始扫：

```http
Host:img.mu
```

但这时它扫到 `m_read_idx` 末尾，也找不到 `\r\n`。

于是：

```cpp
return LINE_OPEN;
```

这表示：

> 当前这一行还没收完整

### 第 4 步：`process_read()` 结束本轮解析

因为 `parse_line()` 返回的不是 `LINE_OK`，所以 `while` 退出。

最后：

```cpp
return NO_REQUEST;
```

### 第 5 步：上层重新监听 `EPOLLIN`

`http_conn::process()` 收到 `NO_REQUEST` 后：

```cpp
modfd(m_epollfd, m_sockfd, EPOLLIN, m_TRIGMode);
return;
```

这一步之后：

- 连接不会关闭
- 缓冲区里的半包不会丢
- 状态机会停在当前进度

### 第 6 步：第二批数据到来

假设下一次又收到：

```http
kewang.com\r\n
User-Agent:...\r\n
\r\n
```

这次 `read_once()` 不是从头写缓冲区，而是接着：

```cpp
m_read_buf + m_read_idx
```

继续往后追加。

所以旧数据：

```http
Host:img.mu
```

会和新数据：

```http
kewang.com\r\n
```

拼成完整的：

```http
Host:img.mukewang.com\r\n
```

### 第 7 步：下次 `process_read()` 从断点继续

这时特别关键：

- `m_read_idx` 变大了
- `m_checked_idx` 还停留在上次没解析完那一行的起点附近
- `m_start_line` 也还保留着那一行的起始位置

所以状态机不是重头解析，而是从上次没完成的那一行继续扫。

这就是它能处理“半个请求头”的根本原因。

---

## 情况二：请求头完整，但请求体只到了一半

### 例子

第一次收到：

```http
POST /login HTTP/1.1\r\n
Host:example.com\r\n
Content-Length:20\r\n
\r\n
user=tom&pa
```

注意：

- 请求头已经完整
- 空行也到了
- body 理论上应该有 20 字节
- 但现在只到了 `user=tom&pa`，明显不够

### 第 1 步：请求行和请求头正常解析

状态机会顺利走到：

```cpp
m_check_state = CHECK_STATE_CONTENT
```

因为空行出现时，`m_content_length != 0`。

### 第 2 步：进入 `parse_content(text)`

这时 `text` 指向 body 起始位置。

它判断：

```cpp
if (m_read_idx >= (m_content_length + m_checked_idx))
```

如果 body 还没收全，这个条件就不成立。

于是：

```cpp
return NO_REQUEST;
```

### 第 3 步：为什么不会误处理

因为源码只有在 body 字节数完全够时，才会：

```cpp
text[m_content_length] = '\0';
m_string = text;
return GET_REQUEST;
```

也就是说：

- 没收完整，绝不会把半个 body 当成完整表单去解析
- 不会提前进入 `do_request()`

这就是通过 `Content-Length` 避免误处理的关键。

### 第 4 步：下一批 body 到来

第二次读到剩余数据后，`m_read_idx` 继续变大。

再次进入 `process_read()` 时：

- 主状态机还停在 `CHECK_STATE_CONTENT`
- 它不会重走请求行和请求头
- 而是直接继续检查 body 是否够长

够长后才正式完成请求。

---

## 情况三：一次来了两个请求，为什么不会乱

### 例子

一次 TCP 读到了：

```http
GET /a HTTP/1.1\r\n
Host:x\r\n
\r\n
GET /b HTTP/1.1\r\n
Host:x\r\n
\r\n
```

这就是典型的粘包。

### 这份代码能做什么

它能正确把第一个请求解析完整。

因为：

- `parse_line()` 能一行一行切
- `process_read()` 会在第一个请求完整后 `return do_request()`

### 但它不是“单次 process 里连续处理多个请求”

也就是说，这个项目不是在一次 `process_read()` 里把两个 HTTP 请求都处理掉。

它更偏向：

1. 先完成当前这个请求
2. 组织响应
3. 如果是 keep-alive，再通过下一轮事件继续处理后面的请求

所以你面试时可以说：

> 这个项目解决了 TCP 流式传输下的半包问题和基本粘包切分问题，但它的请求处理粒度仍然是“一轮先处理一个完整请求”，不是一次函数调用里批量消费多个 HTTP 请求。

这句话是比较稳的。

---

## 为什么半包不会丢：核心原因总结

### 原因 1：新数据不是覆盖写，而是追加写

`read_once()` 读取位置是：

```cpp
recv(m_sockfd, m_read_buf + m_read_idx, READ_BUFFER_SIZE - m_read_idx, 0);
```

重点在：

```cpp
m_read_buf + m_read_idx
```

也就是：

新数据总是接在旧数据后面。

### 原因 2：解析位置有断点

状态机不会每次从头瞎扫，而是靠：

- `m_checked_idx`
- `m_start_line`

记住“上次扫到哪里了”。

### 原因 3：不完整就返回 `NO_REQUEST`

这不是失败，而是“暂缓处理”。

### 原因 4：重新注册读事件

上层收到 `NO_REQUEST` 后，会继续监听这个连接的 `EPOLLIN`。

所以后续数据一来，就能继续补全。

---

## 用一句话解释“等待完整协议”的本质

你可以这样说：

> TinyWebServer 不是在用户态阻塞等待 HTTP 请求完整，而是把当前已经收到的半包保存在读缓冲区里，用 `m_read_idx`、`m_checked_idx`、`m_start_line` 记录读入和解析进度；如果状态机发现请求还不完整，就返回 `NO_REQUEST`，并重新监听 `EPOLLIN`，等下一批数据到来后从断点继续解析。

---

## 你面试时最值得背的 3 句回答

1. TCP 是字节流，应用层看不到天然的请求边界，所以必须自己维护缓冲区和解析状态。
2. 这个项目通过 `m_read_idx` 记录已读字节数，通过 `m_checked_idx` 和 `m_start_line` 记录解析进度，不完整时返回 `NO_REQUEST`，不会丢掉半包。
3. 下次 `EPOLLIN` 到来时，新数据继续追加到旧数据后面，状态机从上次停下的位置继续检查，直到请求头或请求体真正完整。
