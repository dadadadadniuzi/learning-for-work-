---
title: myweb首页随机5张图片全流程模拟
aliases:
  - Workflow myweb GET根路径全流程
  - myweb首页Redis随机图库流程
  - Workflow Redis SRANDMEMBER流程模拟
tags:
  - workflow
  - myweb
  - http
  - redis
  - serieswork
  - cpp
created: 2026-06-08
---

# Workflow-13 myweb首页随机5张图片全流程模拟

关联笔记：
- [[Workflow-04 HTTP 请求与服务端链路]]
- [[Workflow-05 核心抽象：SubTask、SeriesWork、ParallelWork、Workflow]]
- [[Workflow-06 kernel 层：Communicator 与 CommScheduler]]
- [[Workflow-07 上层模块：WFTaskFactory、Server、Client]]
- [[Workflow-12 随机图库面试项目规划]]
- [[Workflow项目面试拆解]]

## 这篇笔记讲什么

这篇笔记只模拟一件事：

> 用户在浏览器访问 `http://127.0.0.1:8888/`，myweb 服务端如何通过 Workflow 接收 HTTP 请求、创建 Redis 异步任务、从 Redis 集合随机取出 5 个图片文件名，最后拼出一个 HTML 页面返回给浏览器。

这篇重点是：

1. Workflow HTTP 服务端是怎么把请求交给我们的 `process(WFHttpTask *task)` 的。
2. `WFRedisTask` 是怎么被创建、设置命令、追加到当前 HTTP 请求的 `SeriesWork` 里的。
3. Redis 内部为什么能随机返回 5 个文件名。
4. Redis 回调完成后，HTML 响应是怎么写回浏览器的。

---

## 本次模拟的固定场景

### 启动命令

假设我们这样启动 myweb：

```bash
cd /home/avavaava/workspace/myweb
./build/myweb 8888 redis://127.0.0.1:6379 /home/avavaava/workspace/myweb/gallery/images myweb:images
```

对应 `main.cpp` 中的参数：

| 参数 | 值 | 含义 |
|---|---|---|
| `argv[1]` | `8888` | HTTP 服务端口 |
| `argv[2]` | `redis://127.0.0.1:6379` | Redis 地址 |
| `argv[3]` | `/home/avavaava/workspace/myweb/gallery/images` | 本地图库目录 |
| `argv[4]` | `myweb:images` | Redis 集合名 |

### Redis 中的数据

启动前我们运行过：

```bash
./scripts/load_gallery.sh redis://127.0.0.1:6379 /home/avavaava/workspace/myweb/gallery/images myweb:images
```

脚本会做两件事：

1. 清空旧集合：`DEL myweb:images`
2. 扫描 `gallery/images` 下的图片文件，把文件名写入 Redis 集合：

```bash
SADD myweb:images 1.jpg
SADD myweb:images 2.jpg
SADD myweb:images 3.gif
SADD myweb:images 4.gif
SADD myweb:images 5.gif
SADD myweb:images 6.jpg
SADD myweb:images 7.png
SADD myweb:images 8.jpg
SADD myweb:images 9.png
SADD myweb:images 10.jpg
SADD myweb:images 11.jpg
SADD myweb:images 12.gif
```

此时 Redis 里不是保存图片二进制，而是保存图片文件名：

```text
key: myweb:images
type: set
members:
  1.jpg
  2.jpg
  3.gif
  ...
  12.gif
```

> [!important]
> Redis 只负责“随机抽文件名”。真正的图片文件仍然保存在本地目录 `/home/avavaava/workspace/myweb/gallery/images`。

### 浏览器请求

用户在浏览器地址栏输入：

```text
http://127.0.0.1:8888/
```

浏览器实际发出的 HTTP 请求大致是：

```http
GET / HTTP/1.1
Host: 127.0.0.1:8888
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8
Connection: keep-alive

```

为了模拟方便，假设这次请求一次性完整到达服务端。

---

## 时间线总览

### 上层调用链

```text
浏览器发送 GET /
-> Workflow 底层 poller 发现 socket 可读
-> Communicator 读取并解析 HTTP 请求
-> WFHttpServer 创建 WFHttpTask
-> Workflow 调用用户 process(WFHttpTask *task)
-> MyWebApp::process(task)
-> 判断 uri == "/"
-> 创建 WFRedisTask
-> RedisRequest 设置 SRANDMEMBER myweb:images 5
-> series_of(http_task)->push_back(redis_task)
-> Redis 异步执行完成
-> MyWebApp::on_home_redis(redis_task)
-> RedisValue 得到 5 个文件名
-> 拼接 HTML
-> HttpResponse::append_output_body(html)
-> Workflow 编码 HTTP 响应并写回浏览器
```

### 你要记住的核心

首页请求不是一次函数调用直接完成的，而是被拆成两个异步任务：

```text
任务1：WFHttpTask，处理 GET /
任务2：WFRedisTask，执行 SRANDMEMBER myweb:images 5
```

这两个任务通过当前 HTTP 请求的 `SeriesWork` 串起来。

---

## 第0步：服务端启动后的初始状态

入口代码在 `src/main.cpp`：

```cpp
WFHttpServer server(process);
server.start(port);
wait_group.wait();
```

假设：

| 对象/变量 | 值 |
|---|---|
| `port` | `8888` |
| `server` | 一个 `WFHttpServer` 对象 |
| `process` | lambda，内部调用 `app.process(task)` |
| `wait_group` | 阻塞主线程，防止程序退出 |

`server.start(8888)` 后，Workflow 大致会做：

```text
WFHttpServer::start(8888)
-> WFServerBase::start(...)
-> WFServerBase::init(...)
-> WFGlobal::get_scheduler()
-> CommScheduler::bind(service)
-> Communicator::bind(service)
-> poller 开始监听 listen_fd
```

此时可以想象服务端状态是：

| 状态 | 值 |
|---|---|
| 监听端口 | `8888` |
| 监听 fd | 假设 `100` |
| 当前连接数 | `0` |
| Redis 是否连接 | 还没有连接，只有请求到来时才创建 Redis 任务 |
| 当前 HTTP 请求 | 无 |

---

## 第1步：浏览器连接到 8888

浏览器要发送 `GET /`，首先要建立 TCP 连接。

假设内核 accept 出来的连接是：

| 变量 | 值 |
|---|---|
| `listen_fd` | `100` |
| 新连接 fd | `109` |
| 客户端地址 | `127.0.0.1:53140` |
| 服务端地址 | `127.0.0.1:8888` |

Workflow 底层大致流程：

```text
mpoller 发现 listen_fd 可读
-> Communicator::handle_listen_result()
-> Communicator::accept_conn()
-> 为 fd=109 建立连接对象
-> 为这个连接注册读事件
```

这一段是 Workflow 帮我们完成的。我们的业务代码暂时还没进入。

---

## 第2步：Workflow 读取并解析 HTTP 请求

浏览器把 HTTP 字节流发给服务端：

```http
GET / HTTP/1.1
Host: 127.0.0.1:8888
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8
Connection: keep-alive

```

底层大致会走：

```text
poller 发现 fd=109 可读
-> Communicator 读取 socket 字节
-> HttpRequest::append() 解析 HTTP 报文
-> 判断一个完整 HTTP 请求已经收完
-> 创建/驱动 WFHttpTask
```

此时 `HttpRequest` 中可以理解为：

| 字段 | 值 |
|---|---|
| method | `GET` |
| request_uri | `/` |
| version | `HTTP/1.1` |
| Host | `127.0.0.1:8888` |
| Connection | `keep-alive` |
| body | 空 |

---

## 第3步：进入用户回调 `MyWebApp::process(task)`

Workflow 在请求解析完后调用：

```cpp
app.process(task);
```

对应源码：

```cpp
void MyWebApp::process(WFHttpTask *task)
{
    HttpRequest *req = task->get_req();
    std::string uri = trim_query(req->get_request_uri());
    ...
}
```

进入函数时关键变量：

| 变量 | 值 |
|---|---|
| `task` | 当前 HTTP 请求任务 |
| `req` | `task->get_req()` |
| `resp` | 还没取，之后可用 `task->get_resp()` |
| `req->get_method()` | `"GET"` |
| `req->get_request_uri()` | `"/"` |
| `uri` | `"/"` |

代码先检查：

```cpp
if (req->get_method() == nullptr || strcmp(req->get_method(), "GET") != 0)
```

这次是 `GET`，所以继续。

然后判断：

```cpp
if (uri == "/")
```

成立，说明这是首页请求，要去 Redis 随机拿 5 个文件名。

---

## 第4步：创建 Redis 异步任务

源码：

```cpp
WFRedisTask *redis_task = WFTaskFactory::create_redis_task(
    config_.redis_url,
    config_.retry_max,
    [this](WFRedisTask *redis_task) { this->on_home_redis(redis_task); });
```

进入前关键配置：

| 字段 | 值 |
|---|---|
| `config_.redis_url` | `redis://127.0.0.1:6379` |
| `config_.retry_max` | `0` |
| `config_.redis_set_name` | `myweb:images` |

创建后：

| 变量 | 含义 |
|---|---|
| `redis_task` | 一个还没开始执行的 Redis 客户端任务 |
| `redis_task->get_req()` | Redis 请求对象 |
| `redis_task->get_resp()` | Redis 响应对象，完成后才有结果 |
| callback | Redis 完成后调用 `on_home_redis(redis_task)` |

> [!note]
> `create_redis_task()` 只是创建任务，不是立刻阻塞执行。Workflow 会在任务被放入 series 后调度它。

---

## 第5步：把原始 HTTP task 存到 Redis task 的 `user_data`

源码：

```cpp
redis_task->user_data = task;
```

这一步非常关键。

因为 Redis 回调函数只有一个参数：

```cpp
void MyWebApp::on_home_redis(WFRedisTask *redis_task)
```

它默认只知道 Redis 任务，不知道最初是哪一个 HTTP 请求触发了它。所以我们手动把 HTTP task 存进去。

此时：

| 表达式 | 值 |
|---|---|
| `redis_task->user_data` | 指向原始 `WFHttpTask *task` |
| `static_cast<WFHttpTask *>(redis_task->user_data)` | 后面可以找回 HTTP task |

面试时可以这样说：

> 我用 `user_data` 在 Redis 异步任务和原始 HTTP 请求之间传递上下文。Redis 回调里通过它找回 HTTP response，然后继续写响应。

---

## 第6步：设置 Redis 命令 `SRANDMEMBER`

源码：

```cpp
redis_task->get_req()->set_request("SRANDMEMBER",
                                   { config_.redis_set_name, "5" });
```

展开后就是：

```text
SRANDMEMBER myweb:images 5
```

各参数含义：

| 参数 | 值 | 含义 |
|---|---|---|
| command | `SRANDMEMBER` | 从 set 中随机取成员 |
| argv[0] | `myweb:images` | Redis 集合名 |
| argv[1] | `5` | 随机返回 5 个成员 |

Redis 协议层发送给 Redis server 时，本质是 RESP 数组：

```text
*3
$11
SRANDMEMBER
$12
myweb:images
$1
5
```

`WFRedisTask` 会把这个请求编码成 Redis RESP 协议，再通过 Workflow 的网络层发给 Redis。

---

## 第7步：把 Redis 任务追加到当前 HTTP 请求的 SeriesWork

源码：

```cpp
series_of(task)->push_back(redis_task);
return;
```

这里是 Workflow 的重点。

`task` 是当前 HTTP 请求的 `WFHttpTask`。每个 task 都属于一个 series，`series_of(task)` 可以拿到当前串行任务流。

追加前：

```text
SeriesWork:
  [ WFHttpTask(GET /) ]
```

追加后：

```text
SeriesWork:
  [ WFHttpTask(GET /) ]
  [ WFRedisTask(SRANDMEMBER myweb:images 5) ]
```

含义：

1. 当前 `process()` 函数返回。
2. HTTP 响应不会立刻发送。
3. Workflow 发现当前 series 后面还有 Redis task。
4. Workflow 调度 Redis task 去连接 Redis、发送命令、等待响应。
5. Redis task 完成后，调用 `on_home_redis()`。
6. 整个 series 都完成后，HTTP response 才会被发回浏览器。

这就是 Workflow 里非常重要的思路：

> 不是在回调里阻塞等待 Redis，而是把 Redis 操作变成一个任务，挂到当前请求的任务链后面。

---

## 第8步：Workflow Redis 客户端连接 Redis

`WFRedisTask` 被调度后，Workflow 内部大致做：

```text
WFRedisTask start
-> 根据 redis://127.0.0.1:6379 解析 host/port
-> RouteManager / DNS / EndpointParams
-> CommScheduler 建立到 Redis 的 TCP 连接
-> RedisRequest encode 成 RESP
-> Communicator 写 socket
-> 等待 Redis 响应
```

这一步如果失败，后面 `redis_task->get_state()` 就不会是 `WFT_STATE_SUCCESS`。

常见失败原因：

| 现象 | 原因 |
|---|---|
| `503 Redis unavailable` | Redis 没启动 |
| `503 Redis unavailable` | URL 端口写错，例如 Redis 在 6379，程序写 16379 |
| `503 Redis unavailable` | Redis 只绑定了别的地址 |
| `503 Redis unavailable` | Redis server 崩了或被关闭 |

测试 Redis 是否可用：

```bash
/home/avavaava/.local/redis-src/src/redis-cli -p 6379 PING
```

如果返回：

```text
PONG
```

说明 Redis 服务本身可连接。

---

## 第9步：Redis 内部处理 `SRANDMEMBER myweb:images 5`

Redis 收到命令：

```text
SRANDMEMBER myweb:images 5
```

它内部逻辑可以理解为：

1. 查找 key：`myweb:images`
2. 判断 key 是否存在
3. 判断 key 类型是否是 set
4. 读取集合当前成员数量
5. 随机抽取 5 个成员
6. 按 RESP 数组返回

假设当前集合成员：

```text
1.jpg
2.jpg
3.gif
4.gif
5.gif
6.jpg
7.png
8.jpg
9.png
10.jpg
11.jpg
12.gif
```

这次 Redis 随机抽到：

```text
1.jpg
7.png
12.gif
5.gif
10.jpg
```

返回给 Workflow 的 RESP 大致是：

```text
*5
$5
1.jpg
$5
7.png
$6
12.gif
$5
5.gif
$6
10.jpg
```

> [!note]
> Redis set 是无序集合，`SRANDMEMBER key 5` 每次可能返回不同成员。因为 count 是正数，所以 Redis 会尽量返回不重复的成员。如果集合成员少于 5 个，就只能返回已有数量。

---

## 第10步：Redis 响应回到 Workflow

Redis server 把响应写回 TCP 连接。

Workflow 底层大致流程：

```text
poller 发现 Redis socket 可读
-> Communicator 读取 Redis 响应字节
-> RedisResponse 解析 RESP
-> WFRedisTask 状态变为 WFT_STATE_SUCCESS
-> 调用 Redis callback
```

也就是进入：

```cpp
MyWebApp::on_home_redis(WFRedisTask *redis_task)
```

此时关键变量：

| 变量 | 值 |
|---|---|
| `redis_task->get_state()` | `WFT_STATE_SUCCESS` |
| `redis_task->get_error()` | `0` |
| `redis_task->user_data` | 原始 `WFHttpTask *` |
| Redis 原始结果 | 5 个字符串组成的数组 |

---

## 第11步：进入 `on_home_redis(redis_task)`

源码：

```cpp
WFHttpTask *root_task = static_cast<WFHttpTask *>(redis_task->user_data);
HttpResponse *resp = root_task->get_resp();
RedisValue value;
```

变量变化：

| 变量 | 值 |
|---|---|
| `redis_task` | Redis 异步任务 |
| `root_task` | 最初的 HTTP 首页请求 |
| `resp` | 首页请求对应的 HTTP 响应对象 |
| `value` | 用来接收 Redis 解析结果 |

这里最重要的是：

```cpp
root_task->get_resp()
```

因为最终 HTML 要写回最初那个浏览器请求。

---

## 第12步：检查 Redis 任务状态

源码：

```cpp
if (redis_task->get_state() != WFT_STATE_SUCCESS)
{
    send_text(resp, "503", "Service Unavailable",
              "<html><body><h1>503 Redis unavailable</h1></body></html>");
    return;
}
```

如果 Redis 没问题，这次：

| 表达式 | 值 |
|---|---|
| `redis_task->get_state()` | `WFT_STATE_SUCCESS` |
| 是否进入 503 | 否 |

如果你浏览器看到：

```text
503 Redis unavailable
```

那说明不是 HTML 拼接问题，也不是图片文件问题，而是 Redis task 没成功完成。

---

## 第13步：把 Redis 响应取成 `RedisValue`

源码：

```cpp
redis_task->get_resp()->get_result(value);
```

执行后，`value` 可以理解为：

```text
value = [
  "1.jpg",
  "7.png",
  "12.gif",
  "5.gif",
  "10.jpg"
]
```

检查：

```cpp
if (!value.is_array())
```

这次 Redis 返回数组，所以通过。

此时关键变量：

| 表达式 | 值 |
|---|---|
| `value.is_array()` | `true` |
| `value.arr_size()` | `5` |
| `value[0].string_value()` | `1.jpg` |
| `value[1].string_value()` | `7.png` |

---

## 第14步：拼接 HTML

源码大致是：

```cpp
std::string html;
html += "<!doctype html><html>...";

for (size_t i = 0; i < value.arr_size(); i++)
{
    std::string name = value[i].string_value();
    html += "<img src=\"/image/";
    html += name;
    html += "\">";
}
```

循环前：

| 变量 | 值 |
|---|---|
| `html` | 已包含 HTML 头部和 CSS |
| `value.arr_size()` | `5` |

循环过程：

| i | `name` | 追加的 HTML |
|---:|---|---|
| 0 | `1.jpg` | `<img src="/image/1.jpg">` |
| 1 | `7.png` | `<img src="/image/7.png">` |
| 2 | `12.gif` | `<img src="/image/12.gif">` |
| 3 | `5.gif` | `<img src="/image/5.gif">` |
| 4 | `10.jpg` | `<img src="/image/10.jpg">` |

最终 HTML 中关键部分大致是：

```html
<div class="grid">
  <div class="card"><a href="/image/1.jpg"><img src="/image/1.jpg" alt="1.jpg"></a></div>
  <div class="card"><a href="/image/7.png"><img src="/image/7.png" alt="7.png"></a></div>
  <div class="card"><a href="/image/12.gif"><img src="/image/12.gif" alt="12.gif"></a></div>
  <div class="card"><a href="/image/5.gif"><img src="/image/5.gif" alt="5.gif"></a></div>
  <div class="card"><a href="/image/10.jpg"><img src="/image/10.jpg" alt="10.jpg"></a></div>
</div>
```

---

## 第15步：写入 HTTP 响应

源码：

```cpp
resp->set_http_version("HTTP/1.1");
resp->set_status_code("200");
resp->set_reason_phrase("OK");
resp->add_header_pair("Content-Type", "text/html; charset=UTF-8");
resp->add_header_pair("Cache-Control", "no-store");
resp->append_output_body(html);
```

此时响应对象变成：

```http
HTTP/1.1 200 OK
Content-Type: text/html; charset=UTF-8
Cache-Control: no-store
Content-Length: 自动计算

<!doctype html><html>...</html>
```

注意：这里不是我们手动 `send()` socket。我们只是把响应内容填到 `HttpResponse` 对象里。真正编码和网络发送由 Workflow 完成。

---

## 第16步：Workflow 发送 HTTP 响应

当当前 series 没有后续任务后，Workflow 会进入回复阶段，大致是：

```text
SeriesWork 发现任务全部完成
-> WFServerTask 准备 reply
-> HttpResponse encode 成 HTTP 字节流
-> Communicator 写 socket
-> 浏览器收到 200 OK + HTML
```

浏览器收到 HTML 后，会解析出 5 个 `<img src="/image/...">`，然后继续发起 5 个图片请求。

这一点很关键：

> 首页请求本身并没有返回图片二进制，它只返回一个 HTML 页面。真正的图片内容是在浏览器后续请求 `/image/<文件名>` 时返回的。

---

## 本篇小结

首页请求的核心链路：

```text
GET /
-> MyWebApp::process()
-> create_redis_task()
-> RedisRequest: SRANDMEMBER myweb:images 5
-> series_of(http_task)->push_back(redis_task)
-> Redis 返回 5 个随机文件名
-> on_home_redis()
-> RedisValue 数组转 HTML
-> HttpResponse 返回 HTML
-> 浏览器继续请求 5 张图片
```

面试讲法：

> 首页请求到达后，Workflow 先把 HTTP 请求解析成 `WFHttpTask`，然后调用我的 `process()`。我判断 URI 是 `/` 后，不阻塞访问 Redis，而是用 `WFTaskFactory::create_redis_task()` 创建一个 Redis 异步任务，并把它追加到当前 HTTP 请求的 `SeriesWork`。Redis 执行 `SRANDMEMBER myweb:images 5`，从 set 中随机返回 5 个图片文件名。Redis 回调里我通过 `user_data` 找回原始 HTTP task，拿到 response，拼接包含 5 个 `<img>` 标签的 HTML。series 完成后，Workflow 把 `HttpResponse` 编码并写回浏览器。
