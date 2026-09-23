> 本章译自 libuv 官方文档 api/handle.rst 与 api/request.rst（libuv 1.52）

# 第一部分 `uv_handle_t` — 基础句柄

`uv_handle_t` 是所有 libuv 句柄类型的基类型。

各结构经过对齐处理，因此任何 libuv 句柄都可以被强制转换为 `uv_handle_t`。这里定义的所有 API 函数都适用于任何句柄类型。

libuv 的句柄不可移动。传递给函数的句柄结构指针在所请求操作的持续期间必须保持有效。使用在栈上分配的句柄时要格外小心。

## 数据类型

### `uv_handle_t`

libuv 基础句柄类型。

### `uv_handle_type`

libuv 句柄的种类。

```c
typedef enum {
  UV_UNKNOWN_HANDLE = 0,
  UV_ASYNC,
  UV_CHECK,
  UV_FS_EVENT,
  UV_FS_POLL,
  UV_HANDLE,
  UV_IDLE,
  UV_NAMED_PIPE,
  UV_POLL,
  UV_PREPARE,
  UV_PROCESS,
  UV_STREAM,
  UV_TCP,
  UV_TIMER,
  UV_TTY,
  UV_UDP,
  UV_SIGNAL,
  UV_FILE,
  UV_HANDLE_TYPE_MAX
} uv_handle_type;
```

### `uv_any_handle`

所有句柄类型的联合（union）。

### `void (*uv_alloc_cb)(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)`

传递给 `uv_read_start()` 和 `uv_udp_recv_start()` 的回调的类型定义。用户必须分配内存并填充所提供的 `uv_buf_t` 结构。如果缓冲区的 base 被赋为 NULL 或长度被赋为 0，将在 `uv_udp_recv_cb` 或 `uv_read_cb` 回调中触发 `UV_ENOBUFS` 错误。

每个缓冲区只使用一次，用户有责任在 `uv_udp_recv_cb` 或 `uv_read_cb` 回调中释放它。

会提供一个建议大小（目前大多数情况下为 65536），但它只是一个参考值，与待读取的数据量没有任何关系。用户可以自由分配自己决定的内存量。

例如，采用自定义分配策略（如使用空闲链表、分配池或基于 slab 的分配器）的应用程序，可以决定使用与其现有内存块相匹配的不同大小。

示例：

```c
static void my_alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
  buf->base = malloc(suggested_size);
  buf->len = suggested_size;
}
```

### `void (*uv_close_cb)(uv_handle_t* handle)`

传递给 `uv_close()` 的回调的类型定义。

### 公共成员

#### `uv_loop_t* uv_handle_t.loop`

指向句柄所在 `uv_loop_t` 的指针。只读。

#### `uv_handle_type uv_handle_t.type`

`uv_handle_type`，指示底层句柄的类型。只读。

#### `void* uv_handle_t.data`

用于存放用户自定义任意数据的空间。libuv 不使用该字段。

## API

### `UV_HANDLE_TYPE_MAP(iter_macro)`

该宏会对每一种句柄类型展开为一系列对 `iter_macro` 的调用。`iter_macro` 以两个参数被调用：去掉 `UV_` 前缀后的 `uv_handle_type` 枚举元素名，以及去掉 `uv_` 前缀和 `_t` 后缀后的对应结构体类型名。

### `int uv_is_active(const uv_handle_t* handle)`

如果句柄是活动的则返回非零值，否则返回零。"活动"的含义取决于句柄类型：

- `uv_async_t` 句柄始终是活动的，无法被停用，除非用 uv_close() 将其关闭。

- `uv_pipe_t`、`uv_tcp_t`、`uv_udp_t` 等句柄——基本上任何涉及 I/O 的句柄——在其正在执行涉及 I/O 的操作（如读取、写入、连接、接受新连接等）时是活动的。

- `uv_check_t`、`uv_idle_t`、`uv_timer_t` 等句柄在通过调用 uv_check_start()、uv_idle_start() 等被启动后是活动的。

经验法则：如果类型为 `uv_foo_t` 的句柄有一个 `uv_foo_start()` 函数，那么从该函数被调用的那一刻起它就是活动的；同样，`uv_foo_stop()` 会再次停用该句柄。

### `int uv_is_closing(const uv_handle_t* handle)`

如果句柄正在关闭或已关闭，返回非零值，否则返回零。

> **注意**：此函数只应在句柄初始化之后、关闭回调到来之前使用。

### `void uv_close(uv_handle_t* handle, uv_close_cb close_cb)`

请求关闭句柄。`close_cb` 将在此调用之后被异步调用。在释放内存之前，必须对每个句柄调用此函数。此外，内存只能在 `close_cb` 中或其返回之后释放。

包装文件描述符的句柄会被立即关闭，但 `close_cb` 仍会被推迟到事件循环的下一次迭代。这给了你一个释放与句柄关联的资源的机会。

进行中的请求（如 uv_connect_t 或 uv_write_t）会被取消，其回调将以 status=UV_ECANCELED 被异步调用。

在不需要清理或释放资源的情况下，`close_cb` 可以为 `NULL`。

### `void uv_ref(uv_handle_t* handle)`

引用给定的句柄。引用操作是幂等的，也就是说，如果句柄已被引用，再次调用此函数不会有任何效果。

参见"引用计数"一节。

### `void uv_unref(uv_handle_t* handle)`

取消对给定句柄的引用。引用操作是幂等的，也就是说，如果句柄未被引用，再次调用此函数不会有任何效果。

参见"引用计数"一节。

### `int uv_has_ref(const uv_handle_t* handle)`

如果句柄被引用，返回非零值，否则返回零。

参见"引用计数"一节。

### `size_t uv_handle_size(uv_handle_type type)`

返回给定句柄类型的大小。对于不想了解结构布局的 FFI 绑定编写者很有用。

## 其他 API 函数

以下 API 函数接受一个 `uv_handle_t` 参数，但只对部分句柄类型有效。

### `int uv_send_buffer_size(uv_handle_t* handle, int* value)`

获取或设置操作系统为套接字使用的发送缓冲区大小。

如果 `*value` == 0，则会将 `*value` 设置为当前的发送缓冲区大小。如果 `*value` > 0，则会用 `*value` 设置新的发送缓冲区大小。

成功时返回零，出错时返回负数结果。

此函数适用于 Unix 上的 TCP、pipe 和 UDP 句柄，以及 Windows 上的 TCP 和 UDP 句柄。

> **注意**：Linux 会将大小设置为所设值的两倍，并返回所设原始值的两倍。

### `int uv_recv_buffer_size(uv_handle_t* handle, int* value)`

获取或设置操作系统为套接字使用的接收缓冲区大小。

如果 `*value` == 0，则会将 `*value` 设置为当前的接收缓冲区大小。如果 `*value` > 0，则会用 `*value` 设置新的接收缓冲区大小。

成功时返回零，出错时返回负数结果。

此函数适用于 Unix 上的 TCP、pipe 和 UDP 句柄，以及 Windows 上的 TCP 和 UDP 句柄。

> **注意**：Linux 会将大小设置为所设值的两倍，并返回所设原始值的两倍。

### `int uv_fileno(const uv_handle_t* handle, uv_os_fd_t* fd)`

获取平台相关的等价文件描述符。

支持以下句柄：TCP、pipes、TTY、UDP 和 poll。传入任何其他句柄类型都会以 `UV_EINVAL` 失败。

如果句柄尚未附加文件描述符，或句柄本身已被关闭，此函数将返回 `UV_EBADF`。

> **警告**：使用此函数时要非常小心。libuv 假定自己掌控着该文件描述符，因此对它的任何更改都可能导致故障。

### `uv_loop_t* uv_handle_get_loop(const uv_handle_t* handle)`

返回 `handle->loop`。

> **注意**：自 1.19.0 版本起新增。

### `void* uv_handle_get_data(const uv_handle_t* handle)`

返回 `handle->data`。

> **注意**：自 1.19.0 版本起新增。

### `void uv_handle_set_data(uv_handle_t* handle, void* data)`

将 `handle->data` 设置为 `data`。

> **注意**：自 1.19.0 版本起新增。

### `uv_handle_type uv_handle_get_type(const uv_handle_t* handle)`

返回 `handle->type`。

> **注意**：自 1.19.0 版本起新增。

### `const char* uv_handle_type_name(uv_handle_type type)`

返回给定句柄类型对应结构的名称，例如 `UV_NAMED_PIPE` 对应 `"pipe"`（即 `uv_pipe_t`）。

如果不存在这样的句柄类型，则返回 `NULL`。

> **注意**：自 1.19.0 版本起新增。

## 引用计数

libuv 事件循环（以默认模式运行时）会一直运行，直到不再有活动*且*被引用的句柄。用户可以通过取消引用处于活动状态的句柄来强制循环提前退出，例如在调用 `uv_timer_start()` 之后调用 `uv_unref()`。

句柄可以处于被引用或未引用状态，引用计数方案并不使用计数器，因此这两种操作都是幂等的。

默认情况下，所有句柄在活动时都是被引用的，关于"活动"的更多细节参见 `uv_is_active()` 的说明。

---

# 第二部分 `uv_req_t` — 基础请求

`uv_req_t` 是所有 libuv 请求类型的基类型。

各结构经过对齐处理，因此任何 libuv 请求都可以被强制转换为 `uv_req_t`。这里定义的所有 API 函数都适用于任何请求类型。

## 数据类型

### `uv_req_t`

libuv 基础请求结构。

### `uv_any_req`

所有请求类型的联合（union）。

### `uv_req_type`

libuv 请求的种类。

```c
typedef enum {
    UV_UNKNOWN_REQ = 0,
    UV_REQ,
    UV_CONNECT,
    UV_WRITE,
    UV_SHUTDOWN,
    UV_UDP_SEND,
    UV_FS,
    UV_WORK,
    UV_GETADDRINFO,
    UV_GETNAMEINFO,
    UV_REQ_TYPE_MAX,
} uv_req_type;
```

### 公共成员

#### `void* uv_req_t.data`

用于存放用户自定义任意数据的空间。libuv 不使用该字段。

#### `uv_req_type uv_req_t.type`

`uv_req_type`，指示请求的类型。只读。

## API

### `UV_REQ_TYPE_MAP(iter_macro)`

该宏会对每一种请求类型展开为一系列对 `iter_macro` 的调用。`iter_macro` 以两个参数被调用：去掉 `UV_` 前缀后的 `uv_req_type` 枚举元素名，以及去掉 `uv_` 前缀和 `_t` 后缀后的对应结构体类型名。

### `int uv_cancel(uv_req_t* req)`

取消一个尚未完成的请求。如果请求正在执行或已执行完毕，则失败。写请求是个例外：取消一个已经在进行中的写操作会尝试中断它（回调可能报告部分写入），而取消一个已完成的写操作则是一次成功的空操作（no-op）。

成功时返回 0，失败时返回小于 0 的错误码。

目前仅支持取消 `uv_write_t`、`uv_fs_t`、`uv_getaddrinfo_t`、`uv_getnameinfo_t`、`uv_random_t` 和 `uv_work_t` 请求。

被取消的请求的回调会在将来某个时刻被调用。在回调被调用之前，释放与请求关联的内存是**不**安全的。

取消通过以下方式报告给回调：

* `uv_fs_t` 请求的 req->result 字段被设置为 `UV_ECANCELED`。

* `uv_work_t`、`uv_getaddrinfo_t`、`uv_getnameinfo_t` 或 `uv_random_t` 请求的回调以 status == `UV_ECANCELED` 被调用。

* `uv_write_t` 请求的回调以 status == `UV_ECANCELED` 被调用。可在回调中使用 `uv_write_nwritten()` 来确定在取消之前已写入多少字节。被完全取消的写入（未写入任何字节）的回调，相对于同一流上的其他写操作可能会乱序调用。注意，如果内核在取消生效之前已完成对该写入的处理，被取消的写入仍可能成功或以其他错误失败。

### `size_t uv_req_size(uv_req_type type)`

返回给定请求类型的大小。对于不想了解结构布局的 FFI 绑定编写者很有用。

### `void* uv_req_get_data(const uv_req_t* req)`

返回 `req->data`。

> **注意**：自 1.19.0 版本起新增。

### `void uv_req_set_data(uv_req_t* req, void* data)`

将 `req->data` 设置为 `data`。

> **注意**：自 1.19.0 版本起新增。

### `uv_req_type uv_req_get_type(const uv_req_t* req)`

返回 `req->type`。

> **注意**：自 1.19.0 版本起新增。

### `const char* uv_req_type_name(uv_req_type type)`

返回给定请求类型对应结构的名称，例如 `UV_CONNECT` 对应 `"connect"`（即 `uv_connect_t`）。

如果不存在这样的请求类型，则返回 `NULL`。

> **注意**：自 1.19.0 版本起新增。
