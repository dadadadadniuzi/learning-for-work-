# Workflow 注释版源码阅读目录

这个目录用于保存 `workflow-master` 的中文注释版源码。

原始源码目录：

```text
G:\计算机学习\workflow项目\workflow-master
```

这里的文件是为了学习和面试复习创建的“阅读版源码”，不会修改原始第三方项目源码。

## 已完成第一批

### `kernel/SubTask.h`

学习重点：

- `SubTask` 是所有任务的最小抽象。
- `dispatch()` 表示启动任务。
- `done()` 表示任务结束后返回下一个任务。
- `subtask_done()` 是任务完成后的统一推进入口。
- `ParallelTask` 是底层并行任务抽象。

### `kernel/SubTask.cc`

学习重点：

- 一个任务完成后如何自动调度下一个任务。
- 并行任务如何用 `nleft` 统计剩余分支。
- 为什么并行分支全部完成后，会回到父任务继续执行。

### `factory/Workflow.h`

学习重点：

- `Workflow` 如何创建串行流和并行流。
- `SeriesWork` 如何表示“先做 A，再做 B”。
- `ParallelWork` 如何表示“多个子流程同时执行”。
- `series_of(task)` 如何通过 `SubTask::pointer` 找回所属流程。

### `factory/Workflow.cc`

学习重点：

- `SeriesWork` 内部循环队列如何管理任务。
- `push_back()` 和 `push_front()` 的区别。
- `pop()` 和 `pop_task()` 如何取出下一个任务。
- `ParallelWork` 如何保存多个 `SeriesWork`。

### `kernel/CommRequest.h`

学习重点：

- `CommRequest` 为什么同时继承 `SubTask` 和 `CommSession`。
- 网络任务如何变成 Workflow 可调度任务。
- `scheduler`、`object`、`target`、`wait_timeout` 分别代表什么。

### `kernel/CommRequest.cc`

学习重点：

- 网络任务完成后如何保存 `state` 和 `error`。
- 如何区分等待连接超时、连接超时和传输超时。
- 为什么最后要调用 `subtask_done()`。

### `kernel/CommScheduler.h`

学习重点：

- `CommScheduler` 是 `Communicator` 上面的连接调度层。
- `CommSchedTarget` 表示单个可调度目标，负责最大连接数和当前负载。
- `CommSchedGroup` 表示一组目标，会按负载选择可用目标。
- `request()` 先 `acquire()` 一个目标，再交给 `Communicator::request()` 做真正网络 IO。
- 它是理解“连接数控制”和“不是一请求一线程”的关键入口。

### `kernel/CommScheduler.cc`

学习重点：

- `CommSchedTarget::acquire()` 会按 `max_connections` 控制单个目标的最大并发连接/请求数。
- 连接数满时，任务可以等待、超时等待或立刻失败。
- `CommSchedTarget::release()` 会释放负载并唤醒等待者。
- `CommSchedGroup` 用小根堆选择负载比例最低的 target。
- 它是解释 Redis/MySQL 高并发连接数控制的核心实现。

### `kernel/Executor.h`

学习重点：

- `Executor` 是计算任务线程池执行器。
- `ExecQueue` 表示任务队列。
- `ExecSession` 定义 `execute()` 和 `handle()`，前者在线程池执行，后者回到任务完成逻辑。
- 图片压缩、缩略图生成这类 CPU 任务可以放到这里。

### `kernel/ExecRequest.h`

学习重点：

- `ExecRequest` 同时继承 `SubTask` 和 `ExecSession`。
- 它把计算任务也纳入 Workflow 编排模型。
- `dispatch()` 提交给 `Executor`，完成后 `handle()` 调用 `subtask_done()`。

### `kernel/Communicator.h`

学习重点：

- `Communicator` 是底层异步通信引擎。
- `CommTarget` 表示远端目标，并维护 idle 连接。
- `CommSession` 表示一次通信会话，提供 `message_out()`、`message_in()`、超时和 `handle()`。
- `CommService` 表示服务端监听。
- `SleepSession` 表示定时器。
- `Communicator` 统一接入 request、reply、bind、sleep、io_bind。
- 它内部使用 `mpoller`、`msgqueue`、`thrdpool` 来支撑事件监听和 handler 线程处理。

### `kernel/Communicator.cc`

学习重点：

- `Communicator::init()` 创建 `mpoller` 和 handler 线程池。
- `request()` 是客户端网络请求入口，会优先复用 idle 连接，失败后才新建非阻塞连接。
- `request_new_conn()` 使用非阻塞 `connect`，并把连接事件交给 `mpoller`。
- `send_message()` 调用协议层 `encode()` 生成 `iovec`，再同步或异步写出。
- 写完请求后会注册读事件等待响应，响应完成后调用 `session->handle()`。
- poller 线程只投递 `poller_result` 到 `msgqueue`，真正回调由 handler 线程处理。
- 这是面试讲“不是一请求一线程，而是事件驱动 + 线程池回调”的核心文件。

### `kernel/poller.h`

学习重点：

- `poller_data` 把 read、write、listen、connect、timer 等事件统一描述。
- `poller_result` 是事件完成后的结果。
- `poller_add()`、`poller_mod()`、`poller_del()` 管理 fd 事件。
- `poller_add_timer()` 管理定时器。
- 它是底层事件循环抽象，Linux 实现通常对应 epoll/timerfd 思路。

### `kernel/mpoller.h`

学习重点：

- `mpoller` 是多个 `poller` 的封装。
- fd 事件按 `fd % nthreads` 分散到不同 poller。
- timer 用轮询计数分散到不同 poller。
- 它解释了多个 poller 线程如何分摊事件监听压力。

### `kernel/thrdpool.c`

学习重点：

- `thrdpool` 是底层 C 线程池。
- 内部用 `msgqueue` 保存待执行任务。
- 工作线程循环 `msgqueue_get()`，然后执行 `task.routine(task.context)`。
- `thrdpool_increase()` / `thrdpool_decrease()` 支持动态增减线程。
- `Executor` 和 `Communicator` 的 handler 线程都依赖类似的线程池思想。

### `manager/WFGlobal.h`

学习重点：

- `WFGlobalSettings` 保存全局线程数、DNS TTL、文件 IO 最大事件数等参数。
- `GLOBAL_SETTINGS_DEFAULT` 展示 Workflow 默认并发配置。
- `WORKFLOW_library_init()` 可以在进程启动早期覆盖默认配置。
- `WFGlobal::get_scheduler()` 是 HTTP、Redis、MySQL 等网络任务进入全局网络调度器的关键入口。
- `WFGlobal::get_compute_executor()`、`get_dns_executor()`、`get_io_service()` 分别对应计算线程池、DNS 线程池和文件 IO 服务。
- 它适合作为面试中解释“Workflow 全局资源如何组织”的入口。

### `manager/EndpointParams.h`

学习重点：

- `EndpointParams` 保存单个网络端点的连接配置。
- `max_connections` 决定同一个目标最多可以建立多少连接，是连接数控制的核心参数。
- `connect_timeout`、`response_timeout`、`ssl_connect_timeout` 分别对应建连、响应和 TLS 握手超时。
- `TransportType` 区分 TCP、UDP、SSL TCP 等传输方式。
- Redis/MySQL/HTTP 客户端任务都会间接使用这些参数。

### `manager/RouteManager.h`

学习重点：

- `RouteManager` 把解析出来的地址转换成 `CommScheduler` 可使用的调度目标。
- `RouteResult` 中的 `request_object` 是后续发起网络请求的关键对象。
- `RouteTarget` 继承 `CommSchedTarget`，因此天然具备最大连接数控制和连接复用能力。
- SSL 场景下会对 `SSL_CTX` 增加引用计数，避免上下文提前释放。
- Redis/MySQL URL 经过 DNS 后，最终会进入这里拿到可复用连接目标。

### `manager/DnsCache.h`

学习重点：

- `DnsCache` 用 `host + port` 作为缓存 key。
- 缓存值包含 `addrinfo`、可信时间和过期时间。
- `get()` / `put()` 返回的是 handle，用完必须 `release()`。
- 删除缓存项时，如果还有任务持有 handle，真正释放会延迟。
- 高并发访问域名形式的 Redis/MySQL 时，DNS 缓存可以减少重复解析。

### `nameservice/WFNameService.h`

学习重点：

- `WFNameService` 是名字服务策略注册表。
- `WFNSPolicy` 是策略接口，默认策略是 `WFDnsResolver`。
- `WFRouterTask` 是名字服务/路由任务，属于 Workflow 任务体系，可以放进串行流。
- `WFNSParams` 封装 URL、传输类型、SSL、固定连接、重试次数等参数。
- 网络任务先经过名字服务得到路由结果，再交给 `CommScheduler` 发起真正 IO。

### `nameservice/WFDnsResolver.h`

学习重点：

- `WFDnsResolver` 是默认 DNS 名字解析策略。
- `WFResolverTask` 会根据 URL host、port、endpoint 参数发起 DNS 或固定地址解析。
- `fixed_conn` 会把 `max_connections` 改成 1，用于固定连接语义。
- DNS 结果会带 TTL 缓存，减少高并发下重复解析压力。
- Redis/MySQL 如果使用域名，就会明显经过这条解析链路。

### `factory/WFTask.h`

学习重点：

- `WFNetworkTask<REQ, RESP>` 是 HTTP、Redis、MySQL 等网络任务共同基类。
- `get_req()` 用来写请求，`get_resp()` 用来读响应。
- `set_send_timeout()`、`set_receive_timeout()`、`set_keep_alive()`、`set_watch_timeout()` 是任务级超时配置。
- `WFFileTask` 是图片读写这类文件 IO 的任务封装。
- `WFTimerTask`、`WFCounterTask`、`WFMailboxTask` 等说明 Workflow 不只包装网络，也包装控制逻辑。

### `factory/WFTask.inl`

学习重点：

- `WFClientTask` 是 HTTP/Redis/MySQL 客户端任务的实际模板实现。
- 客户端任务发送 `req`，接收 `resp`。
- `WFServerTask` 是 HTTP Server 收到请求后的服务端任务模板。
- 服务端任务接收 `req`，发送 `resp`。
- 服务端内部会创建一个特殊 `Series`，流程是 `processor -> reply task`。

### `factory/WFTaskFactory.h`

学习重点：

- `WFTaskFactory` 是用户创建任务最常用的入口。
- `create_redis_task()` 对应随机图库项目里的 Redis 随机集合。
- `create_mysql_task()` 对应图片元数据持久化和查询。
- `create_pread_task()` / `create_pwrite_task()` 对应图片文件读写。
- `WFNetworkTaskFactory` 是更底层的网络任务模板工厂。

### `factory/WFTaskFactory.inl`

学习重点：

- `WFComplexClientTask` 是 HTTP/Redis/MySQL 客户端任务的复杂实现模板。
- 它负责 URL 初始化、端口补全、名字服务、路由、重试、回调切换。
- `WFNetworkTaskFactory::create_client_task()` 最终会创建 `WFComplexClientTask`。
- `WFNetworkTaskFactory::create_server_task()` 会创建 `WFServerTask`，服务端请求从这里进入。
- `create_go_task()` 和 `create_thread_task()` 展示了计算任务如何接入全局线程池。

### `factory/WFTaskFactory.cc`

学习重点：

- `create_timer_task()` 会把 Timer 任务绑定到 `WFGlobal::get_scheduler()`。
- 特殊的 canceled timer 可用于把回调切换到 handler 线程，避免深递归。
- 命名 Timer/Counter/Mailbox/Conditional/Guard 用红黑树和链表按名称管理任务。
- 当前随机图库项目第一阶段不需要深挖命名控制任务。

### `factory/WFConnection.h`

学习重点：

- `WFConnection` 是默认连接对象，继承自 `CommConnection`。
- 每条连接可以挂载一个 `context`，保存连接级状态。
- `context` 使用 `std::atomic<void *>`，支持并发条件设置。
- 析构时可以通过用户提供的 `deleter` 释放上下文。
- 面试讲连接复用时，可以把它和 `RouteTarget`、`CommTarget` 联系起来。

### `factory/WFResourcePool.h`

学习重点：

- `WFResourcePool` 是资源池/令牌池抽象。
- `get()` 获取资源，资源不足时可以让任务等待。
- `post()` 归还资源，并可能唤醒等待任务。
- 内部使用资源数组、等待链表和互斥锁。
- 可以类比成 Workflow 里的任务级并发控制工具。

### `server/WFServer.h`

学习重点：

- `WFServerBase` 负责服务端启动、停止、监听、连接数和超时参数。
- `WFServer<REQ, RESP>` 把协议请求/响应类型和用户 `process` 回调绑定在一起。
- `new_session()` 会在新请求到来时创建一个服务端网络任务。
- 服务端任务会设置 keep-alive、接收超时和请求大小限制。

### `server/WFServer.cc`

学习重点：

- `WFServerBase::start()` 会初始化服务端对象，并绑定到全局 `CommScheduler`。
- `create_listen_fd()` 根据 TCP/UDP/SSL/SCTP 创建监听 socket。
- `new_connection()` 通过 `max_connections` 控制服务端最大连接数。
- `WFServerConnection` 在析构时减少连接计数。
- `shutdown()` 和 `wait_finish()` 用于服务端优雅停止。

### `server/WFHttpServer.h`

学习重点：

- `WFHttpServer` 本质是 `WFServer<HttpRequest, HttpResponse>`。
- 用户传入的 lambda 就是 `http_process_t`。
- `WFHttpServer::new_session()` 会创建 `WFHttpTask`。
- 随机图库项目里的 `/`、`/upload`、`/image/{id}` 都会从这个 process 回调进入。

### `protocol/HttpMessage.h`

学习重点：

- `HttpRequest` 用来读取请求方法、URI 和 body。
- `HttpResponse` 用来设置状态码、header 和输出 body。
- `append_output_body()` 是随机图库项目返回 HTML 或图片内容的关键接口。
- `set_header_pair()` 可以设置 `Content-Type`，比如 `text/html`、`image/jpeg`。
- `HttpMessage` 同时实现了 `ProtocolMessage` 的编码和解析接口。

### `protocol/HttpMessage.cc`

学习重点：

- `append_output_body()` 会拷贝响应 body 数据，适合返回 HTML 或图片内容。
- `append_output_body_nocopy()` 不拷贝数据，但要求外部数据生命周期安全。
- `encode()` 会把 HTTP 起始行、Header、空行和 Body 编成多个 `iovec`。
- `HttpRequest::append()` 负责解析浏览器请求，并处理 `Expect: 100-continue`。
- `size_limit` 可以限制请求体大小，适合上传图片接口防止超大请求。

### `protocol/ProtocolMessage.h`

学习重点：

- `ProtocolMessage` 是 HTTP、Redis、MySQL 等协议消息的共同基础接口。
- `encode()` 负责把消息编码成待发送字节。
- `append()` 负责把收到的字节解析进消息对象。
- `size_limit` 用于限制消息大小，防止异常大包。
- `ProtocolWrapper` 可以包装协议消息，在内外层之间转发编码和解析。

### `protocol/RedisMessage.h`

学习重点：

- `RedisRequest::set_request()` 用来设置 Redis 命令。
- `RedisResponse::get_result()` 用来读取 Redis 返回值。
- `RedisValue` 可以表示 nil、integer、string、status、error、array。
- 随机图库项目可以用 `SRANDMEMBER gallery:images 5` 从 Redis 集合里随机取图片 id。

### `protocol/RedisMessage.cc`

学习重点：

- `RedisValue` 负责把 Redis parser 的底层 reply 转成 C++ 值对象。
- `RedisRequest::set_request()` 会把命令名和参数组成 RESP 数组。
- `RedisMessage::encode_reply()` 按 RESP 协议编码 status、error、integer、string、array。
- `RedisMessage::append()` 负责解析 Redis 返回字节，并检查消息大小限制。
- 随机图库项目中的 `SRANDMEMBER` 命令会经过这条编码路径。

### `protocol/MySQLMessage.h`

学习重点：

- `MySQLRequest::set_query()` 用来设置 SQL。
- `MySQLResponse` 可以判断 OK 包、错误包，读取 affected rows、last insert id、error msg。
- SELECT 结果集的读取通常还要配合 `MySQLResult.h`。
- 随机图库项目中 MySQL 负责保存和查询图片元数据。

### `protocol/MySQLMessage.cc`

学习重点：

- `MySQLRequest::set_query()` 把 SQL 文本包装成 `MYSQL_COM_QUERY` 请求。
- `MySQLMessage::encode()` 会给 payload 加 4 字节 MySQL packet header。
- 超过 MySQL 单包上限的 payload 会被拆成多个 packet。
- `MySQLMessage::append()` 会从字节流中拼出完整 packet，再交给具体消息解析。
- `MySQLResponse` 可以判断 OK 包、ERROR 包，并读取 affected rows、last insert id 等信息。

### `protocol/MySQLResult.h`

学习重点：

- `MySQLResultCursor` 用来遍历 `SELECT` 查询结果。
- `fetch_row(std::vector<MySQLCell>&)` 可以按列下标读取一行。
- `fetch_row(std::map<std::string, MySQLCell>&)` 可以按字段名读取一行。
- `MySQLCell::as_string()`、`as_int()` 等方法把单元格转换成 C++ 类型。
- 随机图库项目查图片路径时会用到它。

## 推荐下一批注释顺序

为了服务随机图库项目，下一批建议按下面顺序继续：

1. `src/factory/WFTaskFactory.cc` 的重点函数二刷
2. `src/factory/WFTask.inl` 的客户端/服务端任务二刷
3. `src/kernel/mpoller.c`
4. `src/kernel/poller.c` 或平台相关 poller 实现
5. `src/kernel/msgqueue.c`

建议不要一次性注释完整个 Workflow。

更好的方式是：

```text
先注释任务编排链路
-> 再注释 HTTP / Redis / MySQL 任务入口
-> 再注释服务端链路
-> 最后注释高并发底层 Communicator / poller / 线程池
```
