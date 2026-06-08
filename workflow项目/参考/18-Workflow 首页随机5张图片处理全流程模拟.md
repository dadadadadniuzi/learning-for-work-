---
title: Workflow 首页随机5张图片处理全流程模拟
aliases:
  - Workflow 随机图库首页请求逐步模拟
  - Workflow GET根路径全链路
tags:
  - workflow
  - http
  - redis
  - mysql
  - serieswork
  - cpp
created: 2026-05-21
---

# 18-Workflow 首页随机5张图片处理全流程模拟

关联笔记：
- [[Workflow项目面试拆解]]
- [[Workflow-04 HTTP 请求与服务端链路]]
- [[Workflow-05 核心抽象：SubTask、SeriesWork、ParallelWork、Workflow]]
- [[Workflow-12 随机图库面试项目规划]]
- [[17-Workflow HTTP获取图片处理全流程模拟]]

## 这篇笔记讲什么

这篇笔记只做一件事：

> 模拟“浏览器访问首页 `/`，服务端随机返回 5 张图片”的完整处理流程

和上一篇 `GET /image/8` 不同，这一篇更适合拿来讲你的面试项目主链路，因为它天然会用到：

```text
WFHttpServer
-> HttpRequest
-> RedisTask
-> MySQLTask
-> SeriesWork
-> HttpResponse
```

也就是说，这次的重点不是“返回某一张图片文件”，而是：

> 一个 HTTP 请求进来后，Workflow 怎么把多个异步任务串起来，最后拼出首页 HTML

---

## 先固定这次模拟的假设

### 请求例子

浏览器请求：

```http
GET / HTTP/1.1
Host: 127.0.0.1:8888
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8
Connection: keep-alive

```

### 业务设计假设

你的随机图库项目首页逻辑大概是：

1. 收到 `GET /`
2. 先查 Redis：

```text
SRANDMEMBER gallery:images 5
```

3. 假设 Redis 返回 5 个随机图片 id：

```text
["8", "3", "12", "21", "5"]
```

4. 再查 MySQL：

```sql
SELECT id, filename, storage_path, content_type
FROM images
WHERE id IN (8, 3, 12, 21, 5);
```

5. 假设查到 5 行图片元数据
6. 服务端拼接一段 HTML：

```html
<img src="/image/8">
<img src="/image/3">
<img src="/image/12">
<img src="/image/21">
<img src="/image/5">
```

7. 最后把这段 HTML 通过 `HttpResponse` 回给浏览器

### 这一篇专门采用“Redis 命中随机集合，MySQL 命中图片元数据”的路径

原因是：

- 这条链最能体现 `SeriesWork`
- 也最适合面试时讲“HTTP -> Redis -> MySQL -> HTML”

---

## 时间线总览

### 整体调用链

```text
浏览器发送 GET /
-> Communicator 收完整 HTTP 请求
-> WFHttpServer::new_session()
-> HttpRequest::append()
-> WFServerTask::handle(WFT_STATE_TOREPLY, 0)
-> Processor::dispatch()
-> 用户 process(WFHttpTask *task)
-> RedisTask: SRANDMEMBER gallery:images 5
-> Redis callback 拿到 5 个 id
-> MySQLTask: SELECT ... WHERE id IN (...)
-> MySQL callback 取出 5 行图片元数据
-> 拼接 HTML
-> HttpResponse::append_output_body()
-> scheduler->reply(this)
-> HttpMessage::encode()
-> Communicator::send_message()
```

建议你把这条链拆成三段去背：

1. **框架收 HTTP 请求**
2. **业务层串 Redis 和 MySQL**
3. **拼 HTML 并回响应**

---

## 第 0 步：服务端已启动，监听 8888

假设服务端启动代码是：

```cpp
WFHttpServer server(process);
server.start(8888);
```

启动后，关键状态和上一篇类似：

| 变量 | 值 |
|---|---|
| `listen_fd` | 假设是 `100` |
| `server.conn_count` | `0` |
| `service->listen_fd` | `100` |
| `service->ref` | `1` |
| `mpoller` | 已经开始监听 `listen_fd=100` |

这里不再重复展开启动过程。

你只需要记住：

> 当浏览器访问 `/` 时，请求仍然会先走 `Communicator -> create_request -> WFHttpServer::new_session -> HttpRequest::append`

---

## 第 1 步：浏览器连接进来，生成 `fd=109`

假设这次首页请求用的是另一个连接：

| 变量 | 值 |
|---|---:|
| `listen_fd` | `100` |
| 新连接 fd | `109` |
| 客户端地址 | `127.0.0.1:53140` |

随后 handler 线程会处理：

```text
handle_poller_result(res)
-> handle_listen_result(res)
-> accept_conn(target, service)
```

### 返回后的连接对象状态

| 变量 | 值 |
|---|---|
| `entry->sockfd` | `109` |
| `entry->seq` | `0` |
| `entry->state` | `CONN_STATE_CONNECTED` |
| `entry->ref` | `1` |
| `server.conn_count` | 从 `0` 变成 `1` |

然后这条连接会被注册成：

| 字段 | 值 |
|---|---|
| `operation` | `PD_OP_READ` |
| `create_message` | `Communicator::create_request` |
| `timeout` | `10000` ms |

意思是：

> 现在开始等待客户端把首页 HTTP 请求发过来

---

## 第 2 步：浏览器发送 `GET /`

假设这次请求一次性完整到达，总长度大约是 `126` 字节。

请求内容：

```http
GET / HTTP/1.1
Host: 127.0.0.1:8888
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8
Connection: keep-alive

```

poller 会把这些字节交给：

```cpp
Communicator::create_request(entry)
```

然后生成这次请求对应的 `WFHttpTask`。

---

## 第 3 步：`create_request()` 创建 `WFHttpTask`

这一段和上一篇相同，本质上会做：

```text
WFHttpServer::new_session(0, entry->conn)
-> WFServerTaskFactory::create_http_task(this, this->process)
```

### 创建后的关键状态

| 变量 | 值 |
|---|---|
| `task` | 当前首页请求对应的 `WFHttpTask` |
| `task->keep_alive_timeout` | `60000` ms |
| `task->receive_timeout` | `-1` |
| `task->req.size_limit` | `size_t(-1)` |
| `session->passive` | `1` |
| `session->seq` | `0` |
| `entry->seq` | 从 `0` 变成 `1` |

接下来：

```cpp
in = session->message_in();
```

对于服务端 HTTP 任务，这里返回的是：

```cpp
&this->req
```

也就是：

> 收到的网络字节会被解析进 `HttpRequest req`

---

## 第 4 步：进入 `HttpRequest::append(buf, &size)` 解析首页请求

### 输入参数

| 变量 | 值 |
|---|---|
| `buf` | 指向完整 GET `/` 请求字节 |
| `*size` | `126` |
| `req.cur_size` | `0` |
| `req.size_limit` | `size_t(-1)` |

调用链：

```text
append_message()
-> HttpRequest::append(buf, &size)
-> HttpMessage::append(buf, &size)
-> http_parser_append_message(...)
```

### 假设解析结果

| 变量 | 值 |
|---|---|
| `*size` | `126` |
| `req.cur_size` | 从 `0` 变成 `126` |
| `req.get_method()` | `"GET"` |
| `req.get_request_uri()` | `"/"` |
| `ret` | `1` |

`ret == 1` 表示：

> 这次首页请求已经完整解析完成

随后 `append_message()` 会做：

| 变量 | 之前 | 之后 |
|---|---|---|
| `entry->state` | `CONN_STATE_RECEIVING` | `CONN_STATE_SUCCESS` |

---

## 第 5 步：进入 `WFServerTask::handle(WFT_STATE_TOREPLY, 0)`

请求收完整后，底层会回调：

```cpp
session->handle(state, error);
```

对于服务端任务来说，关键状态变成：

| 变量 | 值 |
|---|---|
| `state` | `WFT_STATE_TOREPLY` |
| `error` | `0` |

然后走：

```cpp
this->state = WFT_STATE_TOREPLY;
this->target = this->get_target();
new Series(this);
this->processor.dispatch();
```

这里一定要记住：

> 服务端请求并不是“收完就立刻发响应”，而是先创建一个内部 `SeriesWork`，然后启动用户 `process(task)`

这个内部串行流的结构是：

```text
processor
-> server task 自己
```

其中：

- `processor` 负责执行你的业务逻辑
- `server task 自己` 负责最终回复响应

---

## 第 6 步：进入用户 `process(WFHttpTask *task)`

现在真正进入你写的首页业务代码。

我们假设你的 `process` 大概这样：

```cpp
void process(WFHttpTask *task)
{
    std::string uri;
    task->get_req()->get_request_uri(uri); // "/"

    if (uri == "/")
    {
        auto *redis_task = WFTaskFactory::create_redis_task(...);
        redis_task->get_req()->set_request("SRANDMEMBER",
                                           {"gallery:images", "5"});

        series_of(task)->push_back(redis_task);
        return;
    }
}
```

这里有两个关键点：

1. 你先从 `task->get_req()` 取 URI
2. 你没有自己同步等待 Redis，而是把 Redis 任务挂进当前 `SeriesWork`

---

## 第 7 步：从 `HttpRequest` 里取出 URI `/`

业务层会读：

```cpp
task->get_req()->get_request_uri(uri);
```

### 读取结果

| 变量 | 值 |
|---|---|
| `task->get_req()->get_method()` | `"GET"` |
| `task->get_req()->get_request_uri()` | `"/"` |
| `uri` | `"/"` |

然后业务层做路由判断：

```cpp
if (uri == "/")
```

说明：

> 当前请求不是单张图片，不是上传，而是首页随机图接口

---

## 第 8 步：创建 Redis 任务 `SRANDMEMBER gallery:images 5`

可能代码类似：

```cpp
WFRedisTask *redis_task =
    WFTaskFactory::create_redis_task(
        "redis://127.0.0.1:6379",
        0,
        redis_callback);

redis_task->get_req()->set_request(
    "SRANDMEMBER",
    {"gallery:images", "5"});
```

### 关键参数

| 变量 | 值 |
|---|---|
| `url` | `"redis://127.0.0.1:6379"` |
| `retry_max` | `0` |
| `command` | `"SRANDMEMBER"` |
| `params[0]` | `"gallery:images"` |
| `params[1]` | `"5"` |

这一步的实际含义是：

> 让 Redis 从图片集合里随机取 5 个图片 id

---

## 第 9 步：把 Redis 任务挂到当前 `SeriesWork`

这是这篇最重要的一步。

你通常会写：

```cpp
SeriesWork *series = series_of(task);
series->push_back(redis_task);
```

### 这里的参数意义

| 变量 | 值 |
|---|---|
| `task` | 当前首页 HTTP 请求任务 |
| `series` | 这个请求对应的内部串行流 |
| `redis_task` | 下一步要执行的 Redis 网络任务 |

### `push_back(redis_task)` 做了什么

从 `SeriesWork::push_back()` 的角度看：

1. 给 `redis_task` 设置所属 `SeriesWork`
2. 放到当前串行队列尾部
3. 等 `processor` 完成后，串行流自动 `pop()` 出它并执行

也就是说：

> 你不是在 `process(task)` 里“立刻调用 Redis”，而是在告诉 Workflow：等我这个 processor 子任务结束后，请自动去执行 RedisTask

---

## 第 10 步：`process(task)` 返回，串行流自动推进到 RedisTask

`Processor::dispatch()` 执行完你的业务逻辑后会：

```cpp
this->task = NULL;
this->subtask_done();
```

然后 `Processor::done()` 会：

```cpp
return series_of(this)->pop();
```

因为你前面已经：

```cpp
series->push_back(redis_task);
```

所以这时：

| 变量 | 值 |
|---|---|
| `series->next task` | `redis_task` |

也就是说串行流现在会自动转到：

```text
RedisTask::dispatch()
-> CommScheduler::request()
-> Communicator::request()
```

---

## 第 11 步：RedisTask 发起异步网络请求

这一层的链路和前面我们读过的客户端任务一致：

```text
WFRedisTask
-> message_out() 返回 RedisRequest
-> RedisMessage::encode_reply()
-> Communicator::send_message()
-> 等 Redis 响应
-> RedisResponse::append()
```

如果把请求内容展开，发送给 Redis 的逻辑命令就是：

```text
SRANDMEMBER gallery:images 5
```

假设 Redis 返回：

```text
["8", "3", "12", "21", "5"]
```

那么进入 Redis 回调时，关键结果是：

| 变量 | 值 |
|---|---|
| `redis_task->get_state()` | `WFT_STATE_SUCCESS` |
| `redis_task->get_error()` | `0` |
| `redis_result type` | `array` |
| `redis_result size` | `5` |

---

## 第 12 步：Redis 回调里把 5 个图片 id 取出来

假设你的 Redis 回调大概这样：

```cpp
RedisValue value;
redis_task->get_resp()->get_result(value);

std::vector<std::string> ids;
for (size_t i = 0; i < value.arr_size(); i++)
    ids.push_back(value[i].string_value());
```

### 取值后的关键变量

| 变量 | 值 |
|---|---|
| `ids[0]` | `"8"` |
| `ids[1]` | `"3"` |
| `ids[2]` | `"12"` |
| `ids[3]` | `"21"` |
| `ids[4]` | `"5"` |

如果你把它转换成 SQL 片段，那么下一步可能得到：

| 变量 | 值 |
|---|---|
| `id_list_sql` | `"8,3,12,21,5"` |

---

## 第 13 步：在 Redis 回调里创建 MySQL 任务

这一步通常会写：

```cpp
WFMySQLTask *mysql_task =
    WFTaskFactory::create_mysql_task(
        "mysql://user:pass@127.0.0.1:3306/gallery",
        0,
        mysql_callback);
```

然后组织 SQL：

```sql
SELECT id, filename, storage_path, content_type
FROM images
WHERE id IN (8,3,12,21,5);
```

再调用：

```cpp
mysql_task->get_req()->set_query(sql);
```

### 关键参数

| 变量 | 值 |
|---|---|
| `url` | `"mysql://user:pass@127.0.0.1:3306/gallery"` |
| `retry_max` | `0` |
| `sql` | 上面的 `SELECT ... WHERE id IN (...)` |
| `req.command` | `MYSQL_COM_QUERY` |

---

## 第 14 步：把 MySQLTask 继续挂进当前 `SeriesWork`

在 Redis 回调里你通常还会继续这样做：

```cpp
series_of(redis_task)->push_back(mysql_task);
```

注意这里的 `series_of(redis_task)` 和前面的 `series_of(task)` 本质是同一条请求串行流。

### 这一步的意义

它表示：

> Redis 任务完成后，不是你手动同步调用 MySQL，而是继续把 MySQLTask 交给 Workflow 串行推进

于是这条请求内部的任务顺序变成：

```text
processor
-> redis_task
-> mysql_task
-> server task(reply)
```

---

## 第 15 步：MySQLTask 异步发 SQL，并收到结果

MySQL 网络阶段会经过：

```text
MySQLRequest::set_query()
-> MySQLMessage::encode()
-> Communicator::request()
-> 等待 MySQL 响应
-> MySQLResponse::decode_packet()
```

假设 MySQL 成功返回 5 行：

| 列 | 示例值 |
|---|---|
| `id` | `8` |
| `filename` | `cat08.jpg` |
| `storage_path` | `D:/gallery/images/cat08.jpg` |
| `content_type` | `image/jpeg` |

同理还有 `3`、`12`、`21`、`5` 这几行。

这时回调里一般会创建：

```cpp
MySQLResultCursor cursor(mysql_task->get_resp());
```

---

## 第 16 步：MySQL 回调里逐行取结果

可能代码类似：

```cpp
MySQLResultCursor cursor(mysql_task->get_resp());
std::vector<ImageMeta> images;
std::map<std::string, MySQLCell> row;

while (cursor.fetch_row(row))
{
    // 取 id / path / content_type
}
```

### 假设第一次 `fetch_row(row)` 读到

| 字段名 | 值 |
|---|---|
| `row["id"]` | `"8"` |
| `row["filename"]` | `"cat08.jpg"` |
| `row["storage_path"]` | `"D:/gallery/images/cat08.jpg"` |
| `row["content_type"]` | `"image/jpeg"` |

继续循环后，最终 `images` 向量里可能有：

| 下标 | id | path |
|---|---:|---|
| `0` | `8` | `/image/8` |
| `1` | `3` | `/image/3` |
| `2` | `12` | `/image/12` |
| `3` | `21` | `/image/21` |
| `4` | `5` | `/image/5` |

注意：

这里首页 HTML 通常不会直接嵌入图片二进制，而是生成：

```html
<img src="/image/{id}">
```

让浏览器后续再单独请求图片接口。

---

## 第 17 步：在 MySQL 回调里拼接首页 HTML

假设你的 HTML 拼接逻辑类似：

```cpp
std::string html;
html += "<html><body>";
for (auto& img : images)
{
    html += "<img src=\"/image/";
    html += std::to_string(img.id);
    html += "\">";
}
html += "</body></html>";
```

### 拼接后的结果概念上是

```html
<html><body>
<img src="/image/8">
<img src="/image/3">
<img src="/image/12">
<img src="/image/21">
<img src="/image/5">
</body></html>
```

假设这段 HTML 总长度是：

| 变量 | 值 |
|---|---:|
| `html.size()` | `196` |

---

## 第 18 步：开始写 `HttpResponse`

在 MySQL 回调最后，你会把首页 HTML 填进当前 HTTP 请求的响应对象：

```cpp
auto *resp = root_task->get_resp();
resp->set_status_code("200");
resp->set_reason_phrase("OK");
resp->set_header_pair("Content-Type", "text/html; charset=UTF-8");
resp->append_output_body(html);
```

### 响应对象关键参数变化

| 变量 | 变化后 |
|---|---|
| `status_code` | `"200"` |
| `reason_phrase` | `"OK"` |
| `header["Content-Type"]` | `"text/html; charset=UTF-8"` |
| `output_body_size` | `196` |

如果你自己额外设置了：

```cpp
resp->set_header_pair("Connection", "keep-alive");
```

那这个 Header 也会被编码进响应里。

---

## 第 19 步：MySQL 回调结束，串行流回到 server task 自己

MySQL 回调结束后，`mysql_task` 会完成。

它的 `done()` 会做的关键事情是：

```cpp
return series_of(this)->pop();
```

而这个 `SeriesWork` 最后一个任务本来就被设置成：

```text
server task 自己
```

所以现在串行流顺序变成：

```text
processor
-> redis_task
-> mysql_task
-> WFHttpTask 自己（负责 reply）
```

这意味着：

> 前面的 Redis 和 MySQL 都完成了，现在轮到底层正式发 HTTP 响应

---

## 第 20 步：进入 `scheduler->reply(this)`

服务端任务在回复阶段会调用：

```cpp
this->scheduler->reply(this)
```

对当前首页请求来说：

| 变量 | 值 |
|---|---|
| `session` | 当前首页请求对应的 `WFHttpTask` |
| `message_out()` | 返回 `HttpResponse` |
| `entry->sockfd` | `109` |

于是底层进入：

```text
Communicator::reply(session)
-> reply_reliable(session, target)
-> send_message(entry)
```

---

## 第 21 步：进入 `HttpMessage::encode()` 编码首页 HTML 响应

编码前，响应对象里已经有：

| 变量 | 值 |
|---|---|
| `version` | `"HTTP/1.1"` |
| `status_code` | `"200"` |
| `reason_phrase` | `"OK"` |
| `Content-Type` | `"text/html; charset=UTF-8"` |
| `output_body_size` | `196` |

### 编码结果概念上像这样

```text
vectors[0]  -> "HTTP/1.1"
vectors[1]  -> " "
vectors[2]  -> "200"
vectors[3]  -> " "
vectors[4]  -> "OK"
vectors[5]  -> "\r\n"
vectors[6]  -> "Content-Type: text/html; charset=UTF-8\r\n"
vectors[7]  -> "\r\n"
vectors[8]  -> "<html><body> ... 5个img标签 ... </body></html>"
```

如果你额外写了 `Content-Length`，也会单独占一个 header iovec。

---

## 第 22 步：`Communicator::send_message()` 把首页 HTML 回给浏览器

最后发送阶段会走：

```text
send_message(entry)
-> send_message_sync(vectors, cnt, entry)
-> 如果没写完，再 send_message_async(...)
```

因为首页 HTML 很小，这里大概率一次同步写完。

### 写完后的连接状态

因为请求头是：

```http
Connection: keep-alive
```

而服务端 `keep_alive_timeout=60000`，所以：

| 变量 | 值 |
|---|---|
| `entry->state` | `CONN_STATE_KEEPALIVE` 或 `CONN_STATE_IDLE` |
| keep-alive timeout | `60000` ms |

这表示：

> 浏览器后面如果继续请求 `/image/8`，很可能还能复用这条 TCP 连接

---

## 这条首页请求里最关键的参数变化总结

### HTTP 请求完整解析后

| 变量 | 值 |
|---|---|
| `task->get_req()->get_method()` | `"GET"` |
| `task->get_req()->get_request_uri()` | `"/"` |
| `entry->state` | `CONN_STATE_SUCCESS` |

### 业务进入首页路由后

| 变量 | 值 |
|---|---|
| `uri` | `"/"` |
| `route` | 首页随机图逻辑 |

### Redis 返回后

| 变量 | 值 |
|---|---|
| `command` | `SRANDMEMBER gallery:images 5` |
| `ids` | `["8","3","12","21","5"]` |

### MySQL 返回后

| 变量 | 值 |
|---|---|
| `row_count` | `5` |
| `images.size()` | `5` |
| `image ids` | `8, 3, 12, 21, 5` |

### 响应准备完成后

| 变量 | 值 |
|---|---|
| `status_code` | `"200"` |
| `Content-Type` | `"text/html; charset=UTF-8"` |
| `output_body_size` | `196` |

---

## 这篇最值得记住的 3 个面试点

### 1. 首页请求不是一个函数里同步查完 Redis 再查 MySQL

而是：

```text
process(task)
-> create RedisTask
-> push_back 到当前 SeriesWork
-> Redis 回调里再 create MySQLTask
-> 再 push_back 到同一条 SeriesWork
```

这就是 Workflow 最核心的编排思想。

### 2. `SeriesWork` 让多个异步任务看起来像顺序执行

虽然你在代码里写出来像：

```text
先 Redis
再 MySQL
最后写 HTML
```

但底层并没有阻塞线程等待 Redis/MySQL，而是每个任务完成后自动调度下一个。

### 3. 首页 HTML 和图片二进制是两条不同链

这条 `GET /` 请求只负责：

- 随机抽 id
- 查元数据
- 生成 `<img src="/image/{id}">`

真正图片文件本体是在浏览器后续请求 `/image/{id}` 时，才通过另一条链单独返回。

---

## 面试时怎么口语化描述这条链

你可以直接这样说：

> 浏览器访问首页 `/` 后，Workflow 底层先把 HTTP 请求解析成 `WFHttpTask` 和 `HttpRequest`，然后进入我写的 `process(task)`。我会先判断 URI 是 `/`，于是创建一个 RedisTask 执行 `SRANDMEMBER gallery:images 5` 随机取 5 个图片 id，再把这个任务挂到当前请求所属的 `SeriesWork`。Redis 回调回来后，我再创建 MySQLTask 查询这 5 个 id 对应的图片元数据，同样挂到同一条串行流上。MySQL 回调拿到 5 行结果后，拼出首页 HTML，把 5 个 `<img src="/image/{id}">` 标签写进 `HttpResponse`。最后 Workflow 自动回到服务端任务自身，把响应编码并异步发回浏览器。整个过程在代码层看起来像顺序执行，但底层其实是多个异步任务通过 `SeriesWork` 自动推进。

---

## 最后一句话总结

这条首页请求链，本质上可以压缩成一句话：

> Workflow 先把 `GET /` 解析成一个 HTTP 服务端任务，再在用户 `process(task)` 里把 `RedisTask -> MySQLTask -> HTML 拼装` 串成一条 `SeriesWork`，最后由底层异步发回首页响应。
