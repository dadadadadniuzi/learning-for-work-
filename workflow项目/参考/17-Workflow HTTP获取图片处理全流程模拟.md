---
title: Workflow HTTP获取图片处理全流程模拟
aliases:
  - Workflow 服务端图片请求逐步模拟
  - Workflow 获取图片全链路
tags:
  - workflow
  - http
  - server
  - redis
  - mysql
  - fileio
  - cpp
created: 2026-05-21
---

# 17-Workflow HTTP获取图片处理全流程模拟

关联笔记：
- [[Workflow项目面试拆解]]
- [[Workflow-04 HTTP 请求与服务端链路]]
- [[Workflow-06 kernel 层：Communicator 与 CommScheduler]]
- [[Workflow-12 随机图库面试项目规划]]

## 这篇笔记讲什么

这篇笔记只做一件事：

> 模拟“浏览器发来一个获取图片的 HTTP 请求，Workflow 服务端是怎么一步一步把它收进来、解析掉、查到图片、再把图片回给客户端的”

这次我不只讲 HTTP 解析，还会把图片业务链一起串上：

```text
浏览器
-> WFHttpServer
-> HttpRequest 解析
-> 用户 process(task)
-> Redis 查询图片元数据缓存
-> MySQL 回源查图片路径
-> FileIO 读取图片文件
-> HttpResponse 回写图片
```

---

## 先说清楚这次模拟的假设

为了能把每一步参数讲细，这里先固定一个具体场景。

### 请求例子

客户端请求：

```http
GET /image/8 HTTP/1.1
Host: 127.0.0.1:8888
Accept: image/webp,image/*,*/*;q=0.8
Connection: keep-alive

```

### 业务设计假设

我们假设你的面试项目里 `/image/{id}` 的处理逻辑大概是：

1. 从 URI 里取出 `id=8`
2. 先查 Redis：`HGETALL image:meta:8`
3. 这次模拟走**缓存未命中**
4. 再查 MySQL：

```sql
SELECT id, filename, storage_path, content_type, file_size
FROM images
WHERE id = 8
LIMIT 1;
```

5. 假设查到：

| 字段 | 值 |
|---|---|
| `id` | `8` |
| `filename` | `cat08.jpg` |
| `storage_path` | `D:/gallery/images/cat08.jpg` |
| `content_type` | `image/jpeg` |
| `file_size` | `183245` |

6. 然后通过 FileIO 把 `cat08.jpg` 读出来
7. 最后返回：

```http
HTTP/1.1 200 OK
Content-Type: image/jpeg
Content-Length: 183245
Connection: keep-alive
```

---

## 服务端启动时的初始参数

先记住，`WFHttpServer` 默认参数来自 `HTTP_SERVER_PARAMS_DEFAULT`：

| 参数 | 值 |
|---|---:|
| `transport_type` | `TT_TCP` |
| `max_connections` | `2000` |
| `peer_response_timeout` | `10000` ms |
| `receive_timeout` | `-1` |
| `keep_alive_timeout` | `60000` ms |
| `request_size_limit` | `size_t(-1)` |
| `ssl_accept_timeout` | `10000` ms |

如果你的服务代码是：

```cpp
WFHttpServer server(process);
server.start(8888);
```

那么底层会逐步走到：

```text
WFHttpServer::start(8888)
-> WFServerBase::start(AF_INET, NULL, 8888, NULL, NULL)
-> WFServerBase::init(...)
-> WFGlobal::get_scheduler()
-> CommScheduler::bind(this)
-> Communicator::bind(service)
```

此时服务端关键对象状态大致是：

| 变量 | 值 |
|---|---|
| `server.listen_fd` | 假设是 `100` |
| `server.conn_count` | `0` |
| `service->listen_fd` | `100` |
| `service->ref` | `1` |
| `res->data.operation` | `PD_OP_LISTEN` |

含义是：

- 8888 端口已经开始监听
- 还没有客户端连接
- `listen_fd=100` 已经挂到 `mpoller`

---

## 时间线总览

### 整体调用链

```text
浏览器发送 GET /image/8
-> poller 监听到 listen_fd 可读
-> Communicator::handle_listen_result()
-> Communicator::accept_conn()
-> 注册新连接的 PD_OP_READ
-> Communicator::create_request()
-> WFHttpServer::new_session()
-> HttpRequest::append()
-> WFServerTask::handle(WFT_STATE_TOREPLY, 0)
-> Processor::dispatch()
-> 用户 process(WFHttpTask *task)
-> 创建 Redis/MySQL/FileIO 后续任务
-> 最终回到 task->get_resp()
-> scheduler->reply(this)
-> HttpMessage::encode()
-> Communicator 回写响应
```

这一条链建议你分成两段记：

1. **框架收请求**
2. **业务取图片并回响应**

---

## 第 0 步：客户端连接到 8888 端口

浏览器先和服务端建立 TCP 连接。

假设：

| 变量 | 值 |
|---|---:|
| 服务端监听 fd | `100` |
| 新 accept 出来的连接 fd | `108` |
| 客户端地址 | `127.0.0.1:53124` |

这时 `mpoller` 对 `listen_fd=100` 产生一个监听结果，随后这个结果被丢到 `msgqueue`，再由 handler 线程处理：

```text
poller callback
-> Communicator::callback(res, context)
-> msgqueue_put(res, comm->msgqueue)
-> handler_thread_routine()
-> handle_poller_result(res)
-> handle_listen_result(res)
```

---

## 第 1 步：进入 `Communicator::handle_listen_result(res)`

这里处理的是“监听 socket 收到了一个新连接”。

### 进入前关键参数

假设当前 `res` 大致是：

| 字段 | 值 |
|---|---|
| `res->state` | `PR_ST_SUCCESS` |
| `res->data.operation` | `PD_OP_LISTEN` |
| `res->data.fd` | `100` |
| `res->data.context` | `service` |
| `res->data.result` | 指向新建的 `CommServiceTarget` |

源码主线：

```cpp
target = (CommServiceTarget *)res->data.result;
entry = Communicator::accept_conn(target, service);
```

这里的含义是：

- `target` 代表这条客户端连接的“对端目标”
- `entry` 代表这条连接在 Communicator 里的内部状态对象

---

## 第 2 步：进入 `Communicator::accept_conn(target, service)`

这个函数负责把刚 accept 出来的 fd 包装成内部连接对象。

### 输入参数

| 参数 | 值 |
|---|---|
| `target->sockfd` | `108` |
| `service` | 当前 `WFHttpServer` 对应的 `CommService` |

### 关键代码效果

它会做这些事：

1. 把 `108` 设成非阻塞
2. 分配 `CommConnEntry`
3. 调用 `service->new_connection(108)`
4. 把连接状态初始化

### 返回后的关键变量

假设成功后：

| 变量 | 值 |
|---|---:|
| `entry->sockfd` | `108` |
| `entry->seq` | `0` |
| `entry->service` | `service` |
| `entry->target` | `target` |
| `entry->conn` | 一个新的 `WFServerConnection` |
| `entry->state` | `CONN_STATE_CONNECTED` |
| `entry->ref` | `1` |
| `server.conn_count` | 从 `0` 变成 `1` |

含义是：

- 这条连接现在已经被 Workflow 内部接管
- 还没有创建 HTTP 请求任务
- 只是先有了一条“可继续读请求”的连接

---

## 第 3 步：`handle_listen_result()` 给这条新连接注册读事件

如果不是 SSL，这段核心逻辑会走：

```cpp
res->data.operation = PD_OP_READ;
res->data.create_message = Communicator::create_request;
res->data.message = NULL;
timeout = target->response_timeout;
res->data.fd = entry->sockfd;
res->data.context = entry;
mpoller_add(&res->data, timeout, this->mpoller);
```

### 这一刻参数怎么变

| 变量 | 之前 | 之后 |
|---|---|---|
| `res->data.operation` | `PD_OP_LISTEN` | `PD_OP_READ` |
| `res->data.fd` | `100` | `108` |
| `res->data.context` | `service` | `entry` |
| `res->data.create_message` | `accept` 相关逻辑已经结束 | `Communicator::create_request` |
| `timeout` | 无 | `10000` ms |

含义是：

> 监听 fd 的任务结束了，现在开始监听客户端连接 fd=108 上的 HTTP 请求数据

---

## 第 4 步：浏览器把 HTTP 请求字节发过来

请求内容是：

```http
GET /image/8 HTTP/1.1
Host: 127.0.0.1:8888
Accept: image/webp,image/*,*/*;q=0.8
Connection: keep-alive

```

为了方便模拟，假设这次请求一次性全部到达，长度大约是 `111` 字节。

此时 poller 收到的是：

| 字段 | 值 |
|---|---|
| `fd` | `108` |
| `operation` | `PD_OP_READ` |
| `create_message` | `Communicator::create_request` |

接下来就会调用：

```cpp
Communicator::create_request(entry)
```

---

## 第 5 步：进入 `Communicator::create_request(void *context)`

这个函数负责为“这一次收到的服务端 HTTP 请求”创建真正的 session。

### 输入参数

| 参数 | 值 |
|---|---|
| `context` | `entry` |
| `entry->seq` | `0` |
| `entry->conn` | 当前 TCP 连接对象 |

### 关键代码

```cpp
session = service->new_session(entry->seq, entry->conn);
```

而这里的 `service` 实际就是你的 `WFHttpServer`。

所以它会继续进入：

```text
WFHttpServer::new_session(0, entry->conn)
-> WFServerTaskFactory::create_http_task(this, this->process)
```

---

## 第 6 步：进入 `WFHttpServer::new_session(long long seq, CommConnection *conn)`

这里开始创建一次 HTTP 请求对应的 `WFHttpTask`。

### 输入参数

| 参数 | 值 |
|---|---|
| `seq` | `0` |
| `conn` | 当前连接对象 |

### 函数内部做了什么

```cpp
WFHttpTask *task;
task = WFServerTaskFactory::create_http_task(this, this->process);
task->set_keep_alive(this->params.keep_alive_timeout);
task->set_receive_timeout(this->params.receive_timeout);
task->get_req()->set_size_limit(this->params.request_size_limit);
return task;
```

### 创建后的关键参数

假设：

| 变量 | 值 |
|---|---|
| `task` | 新建的 `WFHttpTask` |
| `task->keep_alive_timeout` | `60000` ms |
| `task->receive_timeout` | `-1` |
| `task->req.size_limit` | `size_t(-1)` |

然后回到 `create_request()`。

---

## 第 7 步：`create_request()` 把连接和 session 绑定起来

回到 `Communicator::create_request()` 后，会继续做：

```cpp
session->passive = 1;
entry->session = session;
session->target = target;
session->conn = entry->conn;
session->seq = entry->seq++;
session->out = NULL;
session->in = NULL;
```

### 参数变化

| 变量 | 变化前 | 变化后 |
|---|---|---|
| `session->passive` | 未初始化 | `1` |
| `entry->session` | `NULL` | 指向 `task` |
| `session->target` | 未初始化 | `target` |
| `session->conn` | 未初始化 | `entry->conn` |
| `session->seq` | 未初始化 | `0` |
| `entry->seq` | `0` | `1` |
| `entry->state` | `CONN_STATE_CONNECTED` | `CONN_STATE_RECEIVING` |

接着：

```cpp
in = session->message_in();
```

对于服务端 `WFServerTask` 来说：

```cpp
message_in() -> return &this->req;
```

也就是说：

> 现在 poller 读到的字节，接下来会被解析进 `HttpRequest req`

---

## 第 8 步：进入 `HttpRequest::append(buf, &size)`

此时 poller 把 `111` 字节 HTTP 请求体交给 `append_message()`：

```cpp
ret = in->append(buf, size);
```

这里的 `in` 实际类型就是：

```cpp
HttpRequest *
```

于是进入：

```text
HttpRequest::append(buf, &size)
-> HttpMessage::append(buf, &size)
-> http_parser_append_message(buf, size, this->parser)
```

### 进入前关键参数

| 变量 | 值 |
|---|---|
| `buf` | 指向完整 HTTP 请求字节流 |
| `*size` | `111` |
| `this->cur_size` | `0` |
| `this->size_limit` | `size_t(-1)` |

### 这一步做了什么

1. HTTP parser 逐字节解析请求行和 header
2. 因为这次没有 body，所以整个请求一次完成
3. `this->cur_size += *size`

### 返回后的关键参数

假设：

| 变量 | 值 |
|---|---:|
| `*size` | `111` |
| `this->cur_size` | 从 `0` 变成 `111` |
| `ret` | `1` |

`ret > 0` 的含义是：

> 一个完整 HTTP 请求已经解析完成

---

## 第 9 步：`append_message()` 看到请求完整，把连接状态改成成功

源码主线：

```cpp
ret = in->append(buf, size);
if (ret > 0)
{
    entry->state = CONN_STATE_SUCCESS;
    timeout = -1;
}
```

### 参数变化

| 变量 | 之前 | 之后 |
|---|---|---|
| `entry->state` | `CONN_STATE_RECEIVING` | `CONN_STATE_SUCCESS` |
| `ret` | `1` | `1` |

这里的意思是：

- 这次请求已经完整收到了
- poller 这一段“读请求”的工作已经结束
- 接下来应该把控制权交还给服务端任务本身

---

## 第 10 步：进入 `handle_read_result()` -> `handle_incoming_request()`

后续 handler 线程会处理这个读结果：

```text
handle_poller_result(res)
-> handle_read_result(res)
-> handle_incoming_request(res)
```

在 `handle_incoming_request()` 里，如果读请求成功：

```cpp
session = entry->session;
state = CS_STATE_TOREPLY;
```

然后最后会调用：

```cpp
session->handle(state, res->error);
```

对于我们这次请求：

| 参数 | 值 |
|---|---|
| `session` | 当前 `WFHttpTask` |
| `state` | `CS_STATE_TOREPLY` |
| `res->error` | `0` |

注意：

这里的 `TOREPLY` 不是“已经发响应了”，而是：

> 请求已经完整收到，服务端现在可以开始执行业务逻辑，并准备最终回复了

---

## 第 11 步：进入 `WFServerTask::handle(state, error)`

这是 Workflow 服务端任务的关键分界点。

源码主线：

```cpp
if (state == WFT_STATE_TOREPLY)
{
    this->state = WFT_STATE_TOREPLY;
    this->target = this->get_target();
    new Series(this);
    this->processor.dispatch();
}
```

### 参数变化

| 变量 | 之前 | 之后 |
|---|---|---|
| `this->state` | 初始通信态 | `WFT_STATE_TOREPLY` |
| `this->target` | `NULL` | 当前连接目标 |
| `Series` | 不存在 | 新建了一个 `processor -> task` 的内部串行流 |

这里最重要的一句是：

```cpp
this->processor.dispatch();
```

也就是说：

> 现在开始正式执行你写的 `process(WFHttpTask *task)` 业务逻辑了

---

## 第 12 步：进入 `Processor::dispatch()`，调用用户 `process(task)`

源码主线：

```cpp
this->process(this->task);
```

这时你写的业务函数终于被调用。

我们假设你写的逻辑大概是：

```cpp
void process(WFHttpTask *task)
{
    std::string uri;
    task->get_req()->get_request_uri(uri);   // "/image/8"
    long long image_id = 8;

    auto *redis_task = WFTaskFactory::create_redis_task(...);
    auto *mysql_task = WFTaskFactory::create_mysql_task(...);
    auto *pread_task = WFTaskFactory::create_pread_task(...);

    // 先 Redis，未命中再 MySQL，再 FileIO，最后回填 resp
}
```

---

## 第 13 步：先从 `HttpRequest` 里取 URI

这一步通常会调用：

```cpp
task->get_req()->get_request_uri(uri);
```

### 进入前

HTTP parser 已经把请求内容存进 `task->req.parser`。

所以：

| 变量 | 值 |
|---|---|
| `task->get_req()->get_method()` | `"GET"` |
| `task->get_req()->get_request_uri()` | `"/image/8"` |
| `task->get_req()->get_http_version()` | `"HTTP/1.1"` |

### 业务层解析后

假设你的路由代码把 URI 解析成：

| 变量 | 值 |
|---|---|
| `uri` | `"/image/8"` |
| `image_id` | `8` |
| `redis_key` | `"image:meta:8"` |

---

## 第 14 步：创建 Redis 任务查缓存

你大概率会写：

```cpp
redis_task = WFTaskFactory::create_redis_task(
    "redis://127.0.0.1:6379",
    0,
    redis_callback);
redis_task->get_req()->set_request("HGETALL", {"image:meta:8"});
```

### 创建后关键参数

| 变量 | 值 |
|---|---|
| `url` | `"redis://127.0.0.1:6379"` |
| `retry_max` | `0` |
| `command` | `"HGETALL"` |
| `params[0]` | `"image:meta:8"` |

这一步的本质是：

- 业务逻辑没有自己去连 Redis
- 而是创建了一个新的异步网络任务
- 这个任务后面会走 `CommScheduler -> Communicator -> RedisMessage`

---

## 第 15 步：Redis 回调里发现缓存未命中

假设 Redis 返回的是空数组或 nil，表示：

| 变量 | 值 |
|---|---|
| `redis_task->get_state()` | `WFT_STATE_SUCCESS` |
| `redis_resp` | nil / empty |
| `cache_hit` | `false` |

于是你的回调会继续创建 MySQL 任务：

```cpp
SELECT id, filename, storage_path, content_type, file_size
FROM images
WHERE id = 8
LIMIT 1;
```

---

## 第 16 步：创建 MySQL 任务查图片元数据

可能代码是：

```cpp
mysql_task = WFTaskFactory::create_mysql_task(
    "mysql://user:pass@127.0.0.1:3306/gallery",
    0,
    mysql_callback);

mysql_task->get_req()->set_query(sql);
```

### 关键参数

| 变量 | 值 |
|---|---|
| `url` | `"mysql://user:pass@127.0.0.1:3306/gallery"` |
| `sql` | 上面的 `SELECT ... WHERE id = 8 LIMIT 1` |
| `task->req.command` | `MYSQL_COM_QUERY` |

这一层往下会进入：

```text
MySQLRequest::set_query()
-> MySQLMessage::encode()
-> Communicator::request()
```

也就是说：

> SQL 不是同步执行，而是又变成了一个异步网络任务

---

## 第 17 步：MySQL 回调里拿到图片路径

假设查询成功，返回 1 行：

| 字段 | 值 |
|---|---|
| `id` | `8` |
| `filename` | `cat08.jpg` |
| `storage_path` | `D:/gallery/images/cat08.jpg` |
| `content_type` | `image/jpeg` |
| `file_size` | `183245` |

于是业务层关键变量变成：

| 变量 | 值 |
|---|---|
| `image_path` | `"D:/gallery/images/cat08.jpg"` |
| `content_type` | `"image/jpeg"` |
| `file_size` | `183245` |

然后业务层会创建一个文件读取任务。

---

## 第 18 步：创建 `pread` 任务读取图片文件

大概率会像这样：

```cpp
pread_task = WFTaskFactory::create_pread_task(
    "D:/gallery/images/cat08.jpg",
    file_buffer,
    183245,
    0,
    pread_callback);
```

### 关键参数

| 参数 | 值 |
|---|---|
| `path` | `D:/gallery/images/cat08.jpg` |
| `buf` | 指向预分配内存 |
| `count` | `183245` |
| `offset` | `0` |

含义是：

- 从文件开头读取
- 一次读取整张图片
- 读完后进入 `pread_callback`

---

## 第 19 步：文件读取回调里开始构造 HTTP 响应

假设 `pread` 成功：

| 变量 | 值 |
|---|---|
| `pread_task->get_state()` | `WFT_STATE_SUCCESS` |
| `nread` | `183245` |

这时你开始写响应：

```cpp
auto *resp = task->get_resp();
resp->set_status_code("200");
resp->set_reason_phrase("OK");
resp->set_header_pair("Content-Type", "image/jpeg");
resp->set_header_pair("Content-Length", "183245");
resp->append_output_body(file_buffer, 183245);
```

### 响应对象参数变化

| 变量 | 变化后 |
|---|---|
| `status_code` | `"200"` |
| `reason_phrase` | `"OK"` |
| `header["Content-Type"]` | `"image/jpeg"` |
| `header["Content-Length"]` | `"183245"` |
| `output_body_size` | `183245` |

此时 `HttpResponse` 已经准备好了。

---

## 第 20 步：用户 `process(task)` 返回，Workflow 开始进入回复阶段

`Processor::dispatch()` 结束后会：

```cpp
this->task = NULL;
this->subtask_done();
```

而 `WFServerTask` 的内部 `Series` 是：

```text
processor -> server task 自己
```

所以接下来会回到服务端任务本身，最终走到：

```cpp
this->scheduler->reply(this)
```

含义是：

> 用户业务逻辑已经做完了，接下来轮到底层把 `HttpResponse` 编码并发给浏览器

---

## 第 21 步：进入 `HttpMessage::encode(vectors, max)`

回复阶段，`message_out()` 对服务端任务返回的是：

```cpp
&this->resp
```

所以发送时实际编码的是 `HttpResponse`。

### 编码前关键参数

| 变量 | 值 |
|---|---|
| `start_line[0]` | `"HTTP/1.1"` |
| `start_line[1]` | `"200"` |
| `start_line[2]` | `"OK"` |
| `output_body_size` | `183245` |

### encode 做了什么

它会按顺序把这些内容放进 `iovec`：

1. 状态行：`HTTP/1.1 200 OK\r\n`
2. 各个 Header
3. 空行 `\r\n`
4. 图片二进制 body

编码后概念上像这样：

```text
vectors[0]  -> "HTTP/1.1"
vectors[1]  -> " "
vectors[2]  -> "200"
vectors[3]  -> " "
vectors[4]  -> "OK"
vectors[5]  -> "\r\n"
vectors[6]  -> "Content-Type: image/jpeg\r\n"
vectors[7]  -> "Content-Length: 183245\r\n"
vectors[8]  -> "\r\n"
vectors[9]  -> 183245 字节图片内容
```

---

## 第 22 步：`Communicator::send_message()` 把响应写回浏览器

接下来会进入：

```text
Communicator::send_message(entry)
-> HttpResponse::encode(...)
-> send_message_sync(...)
-> 如果没一次写完，再 send_message_async(...)
```

### 关键点

如果图片不大、socket 发送缓冲也足够，可能同步阶段一次写完。

如果没写完，就会：

1. 复制剩余 `iovec`
2. 注册 `PD_OP_WRITE`
3. 等 socket 可写时继续发

这就是为什么：

> 图片响应再大，也不是用户线程自己阻塞在那里慢慢 write，而是交给 poller + handler 模型继续推进

---

## 第 23 步：响应发完后是否关闭连接

这次请求头里有：

```http
Connection: keep-alive
```

并且服务端 `keep_alive_timeout = 60000`。

所以发完响应后，连接通常不会立刻关掉，而是进入：

| 变量 | 值 |
|---|---|
| `entry->state` | `CONN_STATE_KEEPALIVE` 或 `CONN_STATE_IDLE` |
| keep-alive 超时 | `60000` ms |

含义是：

- 这条连接还可以继续复用
- 如果浏览器随后再请求 `/image/9`
- 就可能直接复用当前 fd=108

---

## 这条链里最关键的参数变化总结

### 连接建立后

| 变量 | 值 |
|---|---:|
| `entry->sockfd` | `108` |
| `entry->seq` | `0` |
| `entry->state` | `CONN_STATE_CONNECTED` |

### HTTP 请求完整解析后

| 变量 | 值 |
|---|---|
| `task->get_req()->get_method()` | `"GET"` |
| `task->get_req()->get_request_uri()` | `"/image/8"` |
| `entry->state` | `CONN_STATE_SUCCESS` |

### 业务层解析 URI 后

| 变量 | 值 |
|---|---|
| `image_id` | `8` |
| `redis_key` | `"image:meta:8"` |

### MySQL 回源后

| 变量 | 值 |
|---|---|
| `image_path` | `"D:/gallery/images/cat08.jpg"` |
| `content_type` | `"image/jpeg"` |
| `file_size` | `183245` |

### HttpResponse 准备完成后

| 变量 | 值 |
|---|---|
| `status_code` | `"200"` |
| `Content-Type` | `"image/jpeg"` |
| `Content-Length` | `"183245"` |
| `output_body_size` | `183245` |

---

## 如果 Redis 命中，会少走哪几步

如果这次：

```text
HGETALL image:meta:8
```

直接命中了，那么会少掉：

1. MySQL 查询
2. MySQL 协议编码/发送/接收

流程会变成：

```text
HTTP 请求
-> process(task)
-> Redis 命中元数据
-> 直接 pread 读文件
-> HttpResponse 回写
```

所以你面试时可以顺手说一句：

> 我这里专门设计了 Redis 元数据缓存，目的是让热点图片请求少走一次 MySQL 网络往返。

---

## 面试时怎么讲这条链路

你可以直接这样说：

> 浏览器请求 `/image/8` 后，底层 `Communicator` 先通过 listen fd 收到新连接，再把新连接注册成一个读事件。请求字节到来后，`create_request()` 会创建一个 `WFHttpTask`，并把收到的字节解析进 `HttpRequest`。当请求完整后，`WFServerTask::handle()` 会启动服务端内部串行流，真正执行我写的 `process(task)`。在业务层里，我先从 URI 解析出图片 id，再异步查 Redis 缓存，缓存未命中就异步查 MySQL 拿图片路径和类型，然后再发起 FileIO 任务读取图片文件。最后把 `Content-Type`、`Content-Length` 和图片 body 写进 `HttpResponse`，底层再通过 `HttpMessage::encode()` 和 `Communicator::send_message()` 异步回写给客户端。整个过程不是一个请求绑死一个线程，而是 HTTP、Redis、MySQL、文件 IO 都被拆成异步任务，由 poller、handler 线程和 Workflow 串行流共同推进。

---

## 这一篇最值得背的 8 个函数

如果你只背函数名和作用，建议背这 8 个：

1. `Communicator::handle_listen_result()`
   作用：处理新连接，把连接 fd 注册成读事件
2. `Communicator::accept_conn()`
   作用：把 accept 出来的 fd 包装成 `CommConnEntry`
3. `Communicator::create_request()`
   作用：为这次服务端请求创建 `WFHttpTask`
4. `WFHttpServer::new_session()`
   作用：创建 HTTP 服务端任务并设置超时与请求大小限制
5. `HttpRequest::append()`
   作用：把收到的字节解析进 HTTP 请求对象
6. `WFServerTask::handle()`
   作用：请求收完后启动用户 `process(task)`
7. `HttpMessage::encode()`
   作用：把 `HttpResponse` 编码成状态行 + header + body
8. `Communicator::send_message()`
   作用：把编码后的 HTTP 响应异步发给客户端

---

## 最后一句话总结

这条“获取图片”请求链，本质上可以压缩成一句话：

> Workflow 先把浏览器发来的 HTTP 字节解析成 `WFHttpTask + HttpRequest`，再在 `process(task)` 里把 Redis、MySQL、FileIO 编排成一条异步任务链，最后把结果写进 `HttpResponse` 并交回底层异步发送。
