---
title: myweb图片文件返回全流程模拟
aliases:
  - Workflow myweb GET图片全流程
  - myweb单张图片FileIO流程
  - Workflow异步文件读取返回图片流程
tags:
  - workflow
  - myweb
  - http
  - fileio
  - serieswork
  - cpp
created: 2026-06-08
---

# Workflow-14 myweb图片文件返回全流程模拟

关联笔记：
- [[Workflow-13 myweb首页随机5张图片全流程模拟]]
- [[Workflow-04 HTTP 请求与服务端链路]]
- [[Workflow-05 核心抽象：SubTask、SeriesWork、ParallelWork、Workflow]]
- [[Workflow-06 kernel 层：Communicator 与 CommScheduler]]
- [[Workflow-07 上层模块：WFTaskFactory、Server、Client]]
- [[Workflow-12 随机图库面试项目规划]]

## 这篇笔记讲什么

上一篇模拟了首页 `GET /` 如何通过 Redis 随机拿到 5 个文件名，并返回 HTML。

这篇继续模拟：

> 浏览器解析 HTML 后，发现里面有 `<img src="/image/1.jpg">`，于是向 myweb 服务端请求 `/image/1.jpg`。服务端如何通过 Workflow 接收请求、校验文件名、创建异步文件 IO 任务、读取本地图片，最后把图片二进制返回给浏览器。

这篇重点是：

1. 浏览器为什么会在首页之后继续请求 `/image/xxx`。
2. `MyWebApp::process()` 如何区分 `/` 和 `/image/<file>`。
3. `schedule_image()` 如何创建 `WFFileIOTask`。
4. `series_of(task)->push_back(pread_task)` 如何让 HTTP 响应等待文件读取完成。
5. `on_image_pread()` 如何设置 `Content-Type`、`Content-Length` 并返回图片内容。

---

## 本次模拟的固定场景

### 首页返回的 HTML 片段

假设上一篇首页请求中，Redis 随机返回了：

```text
1.jpg
7.png
12.gif
5.gif
10.jpg
```

服务端返回的 HTML 中有：

```html
<img src="/image/1.jpg" alt="1.jpg">
<img src="/image/7.png" alt="7.png">
<img src="/image/12.gif" alt="12.gif">
<img src="/image/5.gif" alt="5.gif">
<img src="/image/10.jpg" alt="10.jpg">
```

浏览器解析 HTML 后，会自动发起图片请求。这里我们只模拟其中一张：

```text
GET /image/1.jpg
```

### 图片文件在本地的位置

启动命令中传入的图库目录是：

```text
/home/avavaava/workspace/myweb/gallery/images
```

所以 `1.jpg` 的真实路径是：

```text
/home/avavaava/workspace/myweb/gallery/images/1.jpg
```

假设这个文件大小是：

```text
169517 bytes
```

---

## 时间线总览

```text
浏览器解析首页 HTML
-> 发现 <img src="/image/1.jpg">
-> 浏览器发送 GET /image/1.jpg
-> Workflow 接收并解析 HTTP 请求
-> WFHttpServer 调用 MyWebApp::process(task)
-> uri 以 /image/ 开头
-> 截取 file_name = "1.jpg"
-> is_safe_name("1.jpg") 安全检查
-> schedule_image(task, "1.jpg")
-> join_path 得到完整路径
-> stat 检查文件存在
-> open 打开图片文件
-> malloc 分配图片缓冲区
-> create_pread_task 创建异步文件读取任务
-> series_of(http_task)->push_back(pread_task)
-> Workflow 执行文件 IO
-> on_image_pread(fio_task)
-> 设置 Content-Type: image/jpeg
-> append_output_body 写入图片二进制
-> Workflow 编码 HTTP 响应并写回浏览器
-> 浏览器显示图片
```

---

## 第0步：浏览器为什么会请求 `/image/1.jpg`

首页响应不是图片，而是 HTML：

```html
<img src="/image/1.jpg" alt="1.jpg">
```

浏览器渲染页面时，遇到 `<img>` 标签，会继续向同一个服务器发请求：

```http
GET /image/1.jpg HTTP/1.1
Host: 127.0.0.1:8888
Accept: image/avif,image/webp,image/apng,image/svg+xml,image/*,*/*;q=0.8
Connection: keep-alive

```

这一次请求和首页请求是两次独立 HTTP 请求。它们可能复用同一个 TCP 连接，也可能新建连接，这取决于浏览器和服务端的 keep-alive 状态。

为了模拟方便，假设这次仍然走同一个连接：

| 变量 | 值 |
|---|---|
| 连接 fd | `109` |
| 请求 URI | `/image/1.jpg` |
| 方法 | `GET` |
| 期望响应类型 | 图片 |

---

## 第1步：Workflow 接收图片 HTTP 请求

Workflow 底层流程和首页类似：

```text
poller 发现 fd=109 可读
-> Communicator 读取 socket 字节
-> HttpRequest::append() 解析 HTTP 报文
-> 判断请求完整
-> 创建/驱动 WFHttpTask
-> 调用用户 process(WFHttpTask *task)
```

此时 `HttpRequest` 中可以理解为：

| 字段 | 值 |
|---|---|
| method | `GET` |
| request_uri | `/image/1.jpg` |
| Host | `127.0.0.1:8888` |
| Accept | `image/...` |
| body | 空 |

---

## 第2步：进入 `MyWebApp::process(task)`

源码：

```cpp
HttpRequest *req = task->get_req();
std::string uri = trim_query(req->get_request_uri());
```

进入后关键变量：

| 变量 | 值 |
|---|---|
| `task` | 当前图片请求对应的 `WFHttpTask` |
| `req` | `task->get_req()` |
| `req->get_method()` | `"GET"` |
| `req->get_request_uri()` | `"/image/1.jpg"` |
| `uri` | `"/image/1.jpg"` |

先检查方法：

```cpp
if (req->get_method() == nullptr || strcmp(req->get_method(), "GET") != 0)
```

这次是 GET，所以通过。

然后判断：

```cpp
if (uri == "/")
```

这次不成立，所以不会访问 Redis。

接着判断：

```cpp
const std::string prefix = "/image/";
if (uri.compare(0, prefix.size(), prefix) == 0)
```

这次成立，因为 `"/image/1.jpg"` 以 `"/image/"` 开头。

> [!important]
> 图片请求不再查 Redis。Redis 只在首页随机选文件名时使用。图片请求直接根据 URL 中的文件名去本地图库目录读文件。

---

## 第3步：截取文件名 `file_name`

源码：

```cpp
std::string file_name = uri.substr(prefix.size());
```

此时：

| 表达式 | 值 |
|---|---|
| `uri` | `/image/1.jpg` |
| `prefix` | `/image/` |
| `prefix.size()` | `7` |
| `file_name` | `1.jpg` |

也就是从 URI 中去掉 `/image/` 前缀，只留下文件名。

---

## 第4步：安全检查 `is_safe_name(file_name)`

源码：

```cpp
if (!is_safe_name(file_name))
{
    send_text(task->get_resp(), "400", "Bad Request", ...);
    return;
}
```

`is_safe_name()` 的规则：

```cpp
if (name.empty()) return false;
if (name.find("..") != std::string::npos) return false;
if (name.find('/') != std::string::npos || name.find('\\') != std::string::npos) return false;
return true;
```

本次：

| 检查项 | 值 | 结果 |
|---|---|---|
| 是否为空 | `false` | 通过 |
| 是否包含 `..` | `false` | 通过 |
| 是否包含 `/` | `false` | 通过 |
| 是否包含 `\` | `false` | 通过 |

所以：

```text
is_safe_name("1.jpg") == true
```

为什么要做这个检查？

如果不检查，用户可能请求：

```text
/image/../../../../etc/passwd
```

那拼接路径后就可能访问图库目录外的系统文件。这个检查就是防路径穿越。

---

## 第5步：进入 `schedule_image(task, "1.jpg")`

源码：

```cpp
this->schedule_image(task, file_name);
```

进入函数时：

| 参数 | 值 |
|---|---|
| `task` | 当前图片 HTTP 请求 |
| `file_name` | `1.jpg` |
| `config_.gallery_root` | `/home/avavaava/workspace/myweb/gallery/images` |

这个函数负责：

> 不阻塞当前线程读文件，而是创建一个 Workflow 文件 IO 异步任务。

---

## 第6步：拼出完整文件路径

源码：

```cpp
std::string full_path = join_path(config_.gallery_root, file_name);
```

参数变化：

| 变量 | 值 |
|---|---|
| `config_.gallery_root` | `/home/avavaava/workspace/myweb/gallery/images` |
| `file_name` | `1.jpg` |
| `full_path` | `/home/avavaava/workspace/myweb/gallery/images/1.jpg` |

`join_path()` 会处理目录末尾有没有 `/` 的情况。

---

## 第7步：用 `stat()` 检查文件

源码：

```cpp
struct stat st;
if (stat(full_path.c_str(), &st) < 0 || !S_ISREG(st.st_mode))
{
    send_text(task->get_resp(), "404", "Not Found", ...);
    return;
}
```

本次假设：

| 表达式 | 值 |
|---|---|
| `stat(...)` | `0` |
| `S_ISREG(st.st_mode)` | `true` |
| `st.st_size` | `169517` |

所以文件存在，并且是普通文件。

如果文件不存在，例如请求 `/image/not_exists.jpg`，这里会直接返回：

```http
HTTP/1.1 404 Not Found
```

---

## 第8步：用 `open()` 打开图片文件

源码：

```cpp
int fd = open(full_path.c_str(), O_RDONLY);
```

本次假设：

| 变量 | 值 |
|---|---|
| `full_path` | `/home/.../gallery/images/1.jpg` |
| `open` flags | `O_RDONLY` |
| `fd` | `116` |

这里还没有读文件内容，只是拿到文件描述符。

如果 `fd < 0`，说明打开失败，会返回：

```http
HTTP/1.1 500 Internal Server Error
```

---

## 第9步：分配图片缓冲区

源码：

```cpp
size_t size = (size_t)st.st_size;
void *buf = size > 0 ? malloc(size) : malloc(1);
```

本次：

| 变量 | 值 |
|---|---|
| `st.st_size` | `169517` |
| `size` | `169517` |
| `buf` | 假设地址 `0x7fxx...` |

这块内存将来会用来存放图片二进制。

> [!note]
> 这个项目为了学习流程，采用“一次把整张图片读入内存”的做法。生产环境如果图片很大，可以考虑分块读、sendfile 或流式响应。

---

## 第10步：创建 `ImageReadContext`

源码：

```cpp
ImageReadContext *ctx = new ImageReadContext{
    task,
    fd,
    buf,
    size,
    guess_content_type(file_name)
};
```

本次 `ctx` 内容：

| 字段 | 值 |
|---|---|
| `ctx->root_task` | 当前 `WFHttpTask *` |
| `ctx->fd` | `116` |
| `ctx->buf` | 图片缓冲区地址 |
| `ctx->size` | `169517` |
| `ctx->content_type` | `image/jpeg` |

为什么需要这个结构体？

因为文件 IO 回调函数只有：

```cpp
on_image_pread(WFFileIOTask *fio_task)
```

它默认不知道：

1. 原来的 HTTP 请求是谁。
2. 文件 fd 是多少。
3. 缓冲区地址是多少。
4. 响应头应该写什么 Content-Type。

所以需要把这些上下文打包起来，挂到 `fio_task->user_data` 上。

---

## 第11步：创建 Workflow 文件 IO 任务

源码：

```cpp
WFFileIOTask *pread_task = WFTaskFactory::create_pread_task(
    fd,
    buf,
    size,
    0,
    [this](WFFileIOTask *fio_task) { this->on_image_pread(fio_task); });
```

参数逐个解释：

| 参数 | 本次值 | 含义 |
|---|---|---|
| `fd` | `116` | 从哪个文件描述符读 |
| `buf` | `0x7fxx...` | 读到哪块内存 |
| `size` | `169517` | 最多读取多少字节 |
| `offset` | `0` | 从文件开头读 |
| `callback` | `on_image_pread` | 读完后进入哪个函数 |

创建后：

| 变量 | 含义 |
|---|---|
| `pread_task` | 一个还没执行的异步文件读取任务 |
| `pread_task->get_args()->fd` | `116` |
| `pread_task->get_args()->buf` | `buf` |
| `pread_task->get_args()->count` | `169517` |
| `pread_task->get_args()->offset` | `0` |

---

## 第12步：把上下文挂到文件任务

源码：

```cpp
pread_task->user_data = ctx;
```

此时：

| 表达式 | 值 |
|---|---|
| `pread_task->user_data` | `ctx` |
| `ctx->root_task` | 当前图片 HTTP 请求 |
| `ctx->fd` | `116` |
| `ctx->content_type` | `image/jpeg` |

和首页 Redis 流程一样，这里也用到了 `user_data`。

区别是：

| 流程 | `user_data` 保存什么 |
|---|---|
| 首页 Redis 流程 | 原始 `WFHttpTask *` |
| 图片 FileIO 流程 | `ImageReadContext *` |

---

## 第13步：追加到当前 HTTP 请求的 SeriesWork

源码：

```cpp
series_of(task)->push_back(pread_task);
```

追加前：

```text
SeriesWork:
  [ WFHttpTask(GET /image/1.jpg) ]
```

追加后：

```text
SeriesWork:
  [ WFHttpTask(GET /image/1.jpg) ]
  [ WFFileIOTask(pread fd=116, size=169517, offset=0) ]
```

这句话的含义：

1. 当前 HTTP 业务处理函数 `process()` 返回。
2. HTTP 响应先不发送。
3. Workflow 继续执行 series 中的 `pread_task`。
4. 文件读完后进入 `on_image_pread()`。
5. 等 `pread_task` 也完成后，整个 HTTP 响应才真正写回浏览器。

这和 TinyWebServer 的线程池模型不同。

TinyWebServer 更像：

```text
主线程 epoll 发现可读
-> 读 socket
-> 投递线程池
-> 工作线程处理 HTTP
```

Workflow 更像：

```text
请求本身是一个 task
-> 后续 Redis/FileIO 也是 task
-> 用 series 把它们串起来
-> 框架调度这些 task
```

---

## 第14步：Workflow 执行异步文件读取

`WFFileIOTask` 被调度后，底层大致流程：

```text
WFFileIOTask start
-> Executor / IOService 接收文件 IO 请求
-> 执行 pread(fd=116, buf, 169517, offset=0)
-> 文件内容进入 buf
-> 设置 task retval
-> 调用用户 callback: on_image_pread(fio_task)
```

这里对我们来说最重要的是：

| 字段 | 成功后的值 |
|---|---|
| `fio_task->get_state()` | `WFT_STATE_SUCCESS` |
| `fio_task->get_retval()` | `169517` |
| `ctx->buf` | 已经装入图片二进制 |

---

## 第15步：进入 `on_image_pread(fio_task)`

源码：

```cpp
ImageReadContext *ctx = static_cast<ImageReadContext *>(fio_task->user_data);
HttpResponse *resp = ctx->root_task->get_resp();
long ret = fio_task->get_retval();
```

变量变化：

| 变量 | 值 |
|---|---|
| `fio_task` | 文件 IO 任务 |
| `ctx` | 第10步创建的上下文 |
| `resp` | 当前图片 HTTP 请求的响应对象 |
| `ret` | `169517` |

这一步重新找回 HTTP 响应对象：

```cpp
ctx->root_task->get_resp()
```

否则文件 IO 回调只知道“文件读完了”，不知道应该把图片返回给哪个 HTTP 请求。

---

## 第16步：关闭文件描述符

源码：

```cpp
close(ctx->fd);
```

此时：

| 变量 | 值 |
|---|---|
| `ctx->fd` | `116` |
| 操作 | 关闭文件描述符 |

不管读取成功还是失败，文件描述符都应该关闭。

---

## 第17步：检查文件 IO 是否成功

源码：

```cpp
if (fio_task->get_state() != WFT_STATE_SUCCESS || ret < 0)
{
    free(ctx->buf);
    delete ctx;
    send_text(resp, "503", "Service Unavailable", ...);
    return;
}
```

本次：

| 表达式 | 值 |
|---|---|
| `fio_task->get_state()` | `WFT_STATE_SUCCESS` |
| `ret` | `169517` |
| `ret < 0` | `false` |

所以读取成功，继续构造图片响应。

如果读取失败，会释放资源并返回：

```http
HTTP/1.1 503 Service Unavailable
```

---

## 第18步：设置图片响应头

源码：

```cpp
resp->set_http_version("HTTP/1.1");
resp->set_status_code("200");
resp->set_reason_phrase("OK");
resp->add_header_pair("Content-Type", ctx->content_type.c_str());
std::string content_length = std::to_string(ret);
resp->add_header_pair("Content-Length", content_length.c_str());
resp->add_header_pair("Cache-Control", "public, max-age=60");
```

本次：

| 响应字段 | 值 |
|---|---|
| HTTP version | `HTTP/1.1` |
| status code | `200` |
| reason phrase | `OK` |
| Content-Type | `image/jpeg` |
| Content-Length | `169517` |
| Cache-Control | `public, max-age=60` |

响应头大致是：

```http
HTTP/1.1 200 OK
Content-Type: image/jpeg
Content-Length: 169517
Cache-Control: public, max-age=60
Connection: Keep-Alive

```

---

## 第19步：写入图片二进制响应体

源码：

```cpp
resp->append_output_body(ctx->buf, (size_t)ret);
```

含义：

| 参数 | 值 | 含义 |
|---|---|---|
| `ctx->buf` | 图片二进制缓冲区 | 响应体内容来源 |
| `(size_t)ret` | `169517` | 写入多少字节 |

执行后，`HttpResponse` 中已经包含：

```text
响应头 + 169517 字节 JPEG 图片内容
```

---

## 第20步：释放内存上下文

源码：

```cpp
free(ctx->buf);
delete ctx;
```

释放对象：

| 资源 | 创建位置 | 释放位置 |
|---|---|---|
| `buf` | `malloc(size)` | `free(ctx->buf)` |
| `ctx` | `new ImageReadContext` | `delete ctx` |
| `fd` | `open()` | `close(ctx->fd)` |

这一点面试里可以主动讲：

> 因为文件 IO 是异步回调，不能把局部变量地址传给回调，所以我用堆对象保存上下文，并在回调结束时释放。

---

## 第21步：Workflow 写回浏览器

当 `on_image_pread()` 返回后，当前 series 已经完成：

```text
SeriesWork:
  [ WFHttpTask(GET /image/1.jpg) ] done
  [ WFFileIOTask(pread) ] done
```

Workflow 接下来大致做：

```text
WFServerTask reply
-> HttpResponse encode
-> Communicator 写 socket
-> 浏览器收到 HTTP 200 + JPEG bytes
-> 浏览器解码并显示图片
```

浏览器最终看到的不是 HTML 文本，而是一张图片。

---

## Redis 在这个图片请求里做了什么

严格来说：`GET /image/1.jpg` 这条请求不访问 Redis。

Redis 的作用发生在上一篇首页流程：

```text
GET /
-> Redis SRANDMEMBER myweb:images 5
-> 得到 1.jpg
-> HTML 中出现 /image/1.jpg
```

到了图片请求阶段，URL 已经带着文件名：

```text
/image/1.jpg
```

因此服务端可以直接去本地图库目录读取文件。

如果你以后把项目升级成“图片 ID 模式”，例如：

```text
/image/123
```

那图片请求阶段也可以再查 Redis：

```text
HGETALL image:meta:123
```

用 Redis 缓存 `id -> filename/path/content_type`，缓存未命中再查 MySQL。但当前 myweb 学习版是“Redis 随机文件名 + 本地文件读取”的最小闭环。

---

## 本篇小结

单张图片请求的核心链路：

```text
GET /image/1.jpg
-> MyWebApp::process()
-> 截取 file_name = 1.jpg
-> is_safe_name() 防路径穿越
-> schedule_image()
-> stat 检查文件
-> open 打开文件
-> malloc 分配缓冲区
-> create_pread_task()
-> series_of(http_task)->push_back(pread_task)
-> Workflow 异步读取文件
-> on_image_pread()
-> 设置 image/jpeg 和 Content-Length
-> append_output_body 写入图片二进制
-> Workflow 写回浏览器
```

面试讲法：

> 浏览器拿到首页 HTML 后，会根据 `<img src="/image/1.jpg">` 自动请求图片。Workflow 把这个请求解析成 `WFHttpTask` 后调用我的 `process()`。我识别到 URI 以 `/image/` 开头，取出文件名并做路径安全检查，然后拼成本地文件路径。接着我不直接阻塞读文件，而是创建 `WFFileIOTask`，用 `create_pread_task()` 异步读取图片，并把它追加到当前 HTTP 请求的 `SeriesWork`。文件读完后进入 `on_image_pread()`，我从 `user_data` 取回上下文，设置 `Content-Type` 和 `Content-Length`，把图片二进制写入 `HttpResponse`。最后 Workflow 负责把响应编码并通过 socket 发回浏览器。
