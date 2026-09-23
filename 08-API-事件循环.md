> 本章译自 libuv 官方文档 api/loop.rst（libuv 1.52）

# `uv_loop_t` — 事件循环

事件循环是 libuv 功能的核心部分。它负责轮询 I/O，并根据不同的事件来源调度回调的执行。

## 数据类型

### `uv_loop_t`

循环数据类型。

### `uv_loop_option`

额外的循环选项。参见 `uv_loop_configure()`。

```c
typedef enum {
    UV_LOOP_BLOCK_SIGNAL = 0,
    UV_METRICS_IDLE_TIME,
    UV_LOOP_USE_IO_URING_SQPOLL
} uv_loop_option;
```

### `uv_run_mode`

与 `uv_run()` 一起使用的运行模式。

```c
typedef enum {
    UV_RUN_DEFAULT = 0,
    UV_RUN_ONCE,
    UV_RUN_NOWAIT
} uv_run_mode;
```

### `void (*uv_walk_cb)(uv_handle_t* handle, void* arg)`

传递给 `uv_walk()` 的回调的类型定义。

### 公共成员

#### `void* uv_loop_t.data`

用于存放用户自定义任意数据的空间。libuv 不使用也不会触碰该字段。

## API

### `int uv_loop_init(uv_loop_t* loop)`

初始化给定的 `uv_loop_t` 结构。

### `int uv_loop_configure(uv_loop_t* loop, uv_loop_option option, ...)`

> **注意**：自 1.0.2 版本起新增。

设置额外的循环选项。除非另有说明，通常应在第一次调用 `uv_run()` 之前调用此函数。

成功时返回 0，失败时返回 UV_E* 错误码。请做好处理 UV_ENOSYS 的准备；它表示当前平台不支持该循环选项。

支持的选项：

- UV_LOOP_BLOCK_SIGNAL：在轮询新事件时阻塞某个信号。`uv_loop_configure()` 的第二个参数是信号编号。

  该操作目前仅针对 SIGPROF 信号实现，用于在使用采样分析器（sampling profiler）时抑制不必要的唤醒。请求其他信号会以 UV_EINVAL 失败。

- UV_METRICS_IDLE_TIME：累计事件循环在事件提供者（event provider）中花费的空闲时间。

  要使用 `uv_metrics_idle_time()`，必须启用此选项。

- UV_LOOP_USE_IO_URING_SQPOLL：启用 SQPOLL 模式的 io_uring 实例来处理异步文件系统操作。

> **注意**：1.39.0 版本变更：新增了 UV_METRICS_IDLE_TIME 选项。

> **注意**：1.49.0 版本变更：新增了 UV_LOOP_USE_IO_URING_SQPOLL 选项。

### `int uv_loop_close(uv_loop_t* loop)`

释放循环的所有内部资源。只有当循环已结束执行、且所有打开的句柄和请求都已关闭时才调用此函数，否则它将返回 UV_EBUSY。在此函数返回之后，用户即可释放为循环分配的内存。

### `uv_loop_t* uv_default_loop(void)`

返回已初始化的默认循环。在分配失败时可能返回 NULL。

此函数只是一种在整个应用程序中拥有一个全局循环的便捷方式，默认循环与用 `uv_loop_init()` 初始化的循环没有任何区别。因此，默认循环可以（也应该）用 `uv_loop_close()` 关闭，以释放与之关联的资源。

> **警告**：此函数不是线程安全的。

### `int uv_run(uv_loop_t* loop, uv_run_mode mode)`

此函数运行事件循环。根据指定的模式，其行为有所不同：

- UV_RUN_DEFAULT：运行事件循环，直到不再有活动和被引用的句柄或请求。如果 `uv_stop()` 被调用且仍有活动的句柄或请求，则返回非零值；其他所有情况下返回零。
- UV_RUN_ONCE：轮询一次 I/O。注意，如果没有待处理的回调，此函数会阻塞。完成时（没有剩余的活动句柄或请求）返回零；如果预期还有更多回调（意味着你将来应在某个时刻再次运行事件循环），则返回非零值。
- UV_RUN_NOWAIT：轮询一次 I/O，但如果没有待处理的回调则不阻塞。完成时（没有剩余的活动句柄或请求）返回零；如果预期还有更多回调（意味着你将来应在某个时刻再次运行事件循环），则返回非零值。

`uv_run()` 不可重入。不得在回调中调用它。

### `int uv_loop_alive(const uv_loop_t* loop)`

如果循环中存在被引用的活动句柄、活动请求或正在关闭的句柄，则返回非零值。

### `void uv_stop(uv_loop_t* loop)`

停止事件循环，使 `uv_run()` 尽快结束。这不会早于下一次循环迭代发生。如果此函数在阻塞等待 I/O 之前被调用，循环在本轮迭代中将不会为 I/O 而阻塞。

### `size_t uv_loop_size(void)`

返回 `uv_loop_t` 结构的大小。对于不想了解结构布局的 FFI 绑定编写者很有用。

### `int uv_backend_fd(const uv_loop_t* loop)`

获取后端文件描述符。仅支持 kqueue、epoll 和 event ports。

此函数可与 `uv_run(loop, UV_RUN_NOWAIT)` 结合使用，在一个线程中轮询、在另一个线程中运行事件循环的回调，示例参见 test/test-embed.c。

> **注意**：将 kqueue 的文件描述符嵌入另一个 kqueue 轮询集在所有平台上并非都可行。添加该文件描述符不算错误，但它永远不会产生事件。

### `int uv_backend_timeout(const uv_loop_t* loop)`

获取轮询超时值。返回值以毫秒为单位，-1 表示无超时。

### `uint64_t uv_now(const uv_loop_t* loop)`

返回当前时间戳（毫秒）。时间戳在事件循环 tick 开始时被缓存，详情及缘由参见 `uv_update_time()`。

该时间戳从某个任意的时间点起单调递增。不要对起始点做任何假设，否则你只会失望。

> **注意**：如果需要亚毫秒级精度，请使用 `uv_hrtime()`。

### `void uv_update_time(uv_loop_t* loop)`

更新事件循环所理解的"现在"。libuv 在事件循环 tick 开始时缓存当前时间，以减少与时间相关的系统调用次数。

通常你不需要调用此函数，除非你的回调会长时间阻塞事件循环——"长时间" somewhat 主观，但大概是 1 毫秒或更久的量级。

### `void uv_walk(uv_loop_t* loop, uv_walk_cb walk_cb, void* arg)`

遍历句柄列表：`walk_cb` 将以给定的 `arg` 被执行。

### `int uv_loop_fork(uv_loop_t* loop)`

> **注意**：自 1.12.0 版本起新增。

在 `fork(2)` 系统调用之后，在子进程中重新初始化所有必需的内核状态。

之前已启动的监视器（watcher）在子进程中会继续保持启动状态。

对于在父进程中创建、且计划在子进程中继续使用的每一个事件循环，都必须显式调用此函数，包括默认循环（即使你在父进程中不再使用它）。此函数必须在子进程中调用 `uv_run()` 或任何其他使用该循环的 API 函数之前调用。否则将导致未定义行为，可能包括父进程和子进程都收到重复事件，或导致子进程异常终止。

如果可行，优先选择在子进程中创建新循环，而不是复用父进程中创建的循环。fork 之后在子进程中创建的新循环不应使用此函数。

此函数在 Windows 上未实现，返回 `UV_ENOSYS`。

> **警告**：此函数是实验性的。它可能包含缺陷，且可能被更改或移除。不保证 API 和 ABI 的稳定性。

> **注意**：在 Mac OS X 上，如果父进程中*任意事件循环*曾使用目录 FS 事件句柄，子进程将无法再使用最高效的 FSEvent 实现。此后子进程中目录 FS 事件句柄的使用将回退到与文件以及在其它基于 kqueue 的系统上相同的实现。

> **警告**：在 AIX 和 SunOS 上，fork 时父进程中已启动的 FS 事件句柄*不会*在子进程中投递事件；它们必须被关闭并重新启动。在所有其他平台上，它们无需任何额外干预即可继续正常工作。

> **警告**：之前从 `uv_backend_fd()` 得到的任何返回值现在都已失效。必须重新调用该函数以确定正确的后端文件描述符。

### `void* uv_loop_get_data(const uv_loop_t* loop)`

返回 `loop->data`。

> **注意**：自 1.19.0 版本起新增。

### `void uv_loop_set_data(uv_loop_t* loop, void* data)`

将 `loop->data` 设置为 `data`。

> **注意**：自 1.19.0 版本起新增。
