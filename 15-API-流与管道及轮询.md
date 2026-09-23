# uv_pipe_t — 管道句柄 · uv_stream_t — 流句柄 · uv_poll_t — 轮询句柄 · uv_tty_t — TTY 句柄

> 本章译自 libuv 官方文档 api/pipe.rst、api/stream.rst、api/poll.rst、api/tty.rst（libuv 1.52）

---

# uv_pipe_t — 管道句柄

管道句柄在 Unix 上（包括本地域套接字、管道和 FIFO）以及 Windows 上的命名管道之上提供了一层抽象。

`uv_pipe_t` 是 `uv_stream_t` 的"子类"。

## 数据类型

### uv_pipe_t

管道句柄类型。

## 公开成员

### int uv_pipe_t.ipc

此管道是否适合用于进程间传递句柄。只有即将传递句柄的已连接管道才应设置此标志，而不是调用 uv_accept 的监听管道。

> **参见**：`uv_stream_t` 的成员同样适用。

## API

### int uv_pipe_init(uv_loop_t* loop, uv_pipe_t* handle, int ipc)

初始化一个管道句柄。`ipc` 参数是一个布尔值，指示此管道是否将用于进程间传递句柄（这可能会改变传输的字节内容）。只有即将传递句柄的已连接管道才应设置此标志，而不是调用 uv_accept 的监听管道。

### int uv_pipe_open(uv_pipe_t* handle, uv_file file)

将现有的文件描述符或 HANDLE 作为管道打开。

*1.2.1 版本变更：文件描述符会被设置为非阻塞模式。*

> **注意**：传入的文件描述符或 HANDLE 不会做类型检查，但要求它必须代表一个有效的管道。

### int uv_pipe_bind(uv_pipe_t* handle, const char* name)

将管道绑定到一个文件路径（Unix）或名称（Windows）。

与 `uv_pipe_bind2` 不同，不支持 Linux 抽象命名空间套接字。

是 `uv_pipe_bind2(handle, name, strlen(name), 0)` 的别名。

> **注意**：Unix 上的路径会被截断到 `sizeof(sockaddr_un.sun_path)` 字节，通常在 92 到 108 字节之间。

### int uv_pipe_bind2(uv_pipe_t* handle, const char* name, size_t namelen, unsigned int flags)

将管道绑定到一个文件路径（Unix）或名称（Windows）。

`flags` 必须为 0 或 `UV_PIPE_NO_TRUNCATE`。遇到不支持的标志时返回 `UV_EINVAL`，不执行绑定操作。

支持 Linux 抽象命名空间套接字。`namelen` 必须包含前导的 nul 字节，但不包含结尾的 nul 字节。

*自 1.46.0 版本新增*

> **注意**：Unix 上的路径会被截断到 `sizeof(sockaddr_un.sun_path)` 字节，通常在 92 到 108 字节之间；除非指定了 `UV_PIPE_NO_TRUNCATE` 标志，此时会返回 `UV_EINVAL` 错误。

### void uv_pipe_connect(uv_connect_t* req, uv_pipe_t* handle, const char* name, uv_connect_cb cb)

连接到 Unix 域套接字或 Windows 命名管道。

与 `uv_pipe_connect2` 不同，不支持 Linux 抽象命名空间套接字。

是 `uv_pipe_connect2(req, handle, name, strlen(name), 0, cb)` 的别名。

> **注意**：Unix 上的路径会被截断到 `sizeof(sockaddr_un.sun_path)` 字节，通常在 92 到 108 字节之间。

### int uv_pipe_connect2(uv_connect_t* req, uv_pipe_t* handle, const char* name, size_t namelen, unsigned int flags, uv_connect_cb cb)

连接到 Unix 域套接字或 Windows 命名管道。

`flags` 必须为 0 或 `UV_PIPE_NO_TRUNCATE`。遇到不支持的标志时返回 `UV_EINVAL`，不执行连接操作。

支持 Linux 抽象命名空间套接字。`namelen` 必须包含前导的 nul 字节，但不包含结尾的 nul 字节。

*自 1.46.0 版本新增*

> **注意**：Unix 上的路径会被截断到 `sizeof(sockaddr_un.sun_path)` 字节，通常在 92 到 108 字节之间；除非指定了 `UV_PIPE_NO_TRUNCATE` 标志，此时会返回 `UV_EINVAL` 错误。

### int uv_pipe_getsockname(const uv_pipe_t* handle, char* buffer, size_t* size)

获取 Unix 域套接字或命名管道的名称。

必须提供一个预先分配好的缓冲区。`size` 参数在输入时保存缓冲区的长度，在输出时被设置为写入缓冲区的字节数。如果缓冲区不够大，将返回 `UV_ENOBUFS`，且 `len` 将包含所需的大小。

FreeBSD 上 UNIX 域套接字的路径不能超过 `sockaddr_un.sun_path` 的大小，尽管该平台定义了更大的 `SOCK_MAXADDRLEN`。

*1.3.0 版本变更：返回的长度不再包含结尾的 null 字节，且缓冲区不以 null 结尾。*

### int uv_pipe_getpeername(const uv_pipe_t* handle, char* buffer, size_t* size)

获取该句柄所连接的 Unix 域套接字或命名管道的名称。

必须提供一个预先分配好的缓冲区。`size` 参数在输入时保存缓冲区的长度，在输出时被设置为写入缓冲区的字节数。如果缓冲区不够大，将返回 `UV_ENOBUFS`，且 `len` 将包含所需的大小。

*自 1.3.0 版本新增*

### void uv_pipe_pending_instances(uv_pipe_t* handle, int count)

设置管道服务器等待连接时的待处理管道实例数。

> **注意**：此设置仅适用于 Windows。

### int uv_pipe_pending_count(uv_pipe_t* handle)
### uv_handle_type uv_pipe_pending_type(uv_pipe_t* handle)

用于通过 IPC 管道接收句柄。

步骤：先调用 `uv_pipe_pending_count`，如果结果 > 0，则按 `uv_pipe_pending_type` 返回的给定 `type` 初始化一个句柄，然后调用 `uv_accept(pipe, handle)`。

> **参见**：`uv_stream_t` 的 API 函数同样适用。

### int uv_pipe_chmod(uv_pipe_t* handle, int flags)

修改管道权限，使其可以被不同用户运行的进程访问。使管道对所有用户可写或可读。模式可以是 `UV_WRITABLE`、`UV_READABLE` 或 `UV_WRITABLE | UV_READABLE`。此函数是阻塞的。

*自 1.16.0 版本新增*

### int uv_pipe(uv_file fds[2], int read_flags, int write_flags)

创建一对已连接的管道句柄。数据可以写入 `fds[1]` 并从 `fds[0]` 读取。得到的句柄可以传给 `uv_pipe_open`、用于 `uv_spawn`，或用于其他任何用途。

`flags` 的有效值为：

- UV_NONBLOCK_PIPE：以 `OVERLAPPED` 或 `FIONBIO`/`O_NONBLOCK` I/O 方式打开指定的句柄。对于将被 libuv 使用的句柄，推荐这样做；其他情况通常不建议。

等价于设置了 `O_CLOEXEC` 标志的 `pipe(2)`。

*自 1.41.0 版本新增*

---

# uv_stream_t — 流句柄

流句柄提供了对双工通信信道的抽象。`uv_stream_t` 是一个抽象类型，libuv 以 `uv_tcp_t`、`uv_pipe_t` 和 `uv_tty_t` 的形式提供了 3 种流的实现。

## 数据类型

### uv_stream_t

流句柄类型。

### uv_connect_t

连接请求类型。

### uv_shutdown_t

关闭（shutdown）请求类型。

### uv_write_t

写请求类型。复用此类型的对象时必须格外小心。当流处于非阻塞模式时，通过 `uv_write` 发出的写请求会被排队。此时复用对象是未定义行为。只有在传给 `uv_write` 的回调被触发之后，才可安全地复用 `uv_write_t` 对象。

### void (*uv_read_cb)(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf)

在流上读到数据时调用的回调。

当有数据可用时 `nread` > 0，出错时 < 0。当到达 EOF 时，`nread` 会被设为 `UV_EOF`。当 `nread` < 0 时，`buf` 参数可能不指向有效的缓冲区；此时 `buf.len` 和 `buf.base` 都会被置为 0。

> **注意**：`nread` 可能为 0，这*不*表示错误或 EOF。这等价于 `read(2)` 下的 `EAGAIN` 或 `EWOULDBLOCK`。

发生错误时，被调用方负责通过调用 `uv_read_stop` 或 `uv_close` 来停止/关闭流。再次尝试从流中读取是未定义的。

被调用方负责释放缓冲区，libuv 不会复用它。出错时缓冲区可能是一个空缓冲区（即 `buf->base` == NULL 且 `buf->len` == 0）。

### void (*uv_write_cb)(uv_write_t* req, int status)

在流上写入数据后调用的回调。成功时 `status` 为 0，否则 < 0。

### void (*uv_connect_cb)(uv_connect_t* req, int status)

由 `uv_connect` 发起的连接完成后调用的回调。成功时 `status` 为 0，否则 < 0。

### void (*uv_shutdown_cb)(uv_shutdown_t* req, int status)

关闭请求完成后调用的回调。成功时 `status` 为 0，否则 < 0。

### void (*uv_connection_cb)(uv_stream_t* server, int status)

流服务器收到传入连接时调用的回调。用户可以通过调用 `uv_accept` 来接受该连接。成功时 `status` 为 0，否则 < 0。

## 公开成员

### size_t uv_stream_t.write_queue_size

包含正在排队等待发送的字节数。只读。

### uv_stream_t* uv_connect_t.handle

此连接请求在其上运行的流的指针。

### uv_stream_t* uv_shutdown_t.handle

此关闭请求在其上运行的流的指针。

### uv_stream_t* uv_write_t.handle

此写请求在其上运行的流的指针。

### uv_stream_t* uv_write_t.send_handle

正通过此写请求发送的流的指针。

> **参见**：`uv_handle_t` 的成员同样适用。

## API

### int uv_shutdown(uv_shutdown_t* req, uv_stream_t* handle, uv_shutdown_cb cb)

关闭双工流的输出（写）一侧。它会等待待处理的写请求完成。`handle` 应指向一个已初始化的流。`req` 应是一个未初始化的关闭请求结构体。关闭完成后会调用 `cb`。

### int uv_listen(uv_stream_t* stream, int backlog, uv_connection_cb cb)

开始监听传入连接。`backlog` 表示内核可以排队的连接数，与 `listen(2)` 相同。当收到新的传入连接时，会调用 `uv_connection_cb` 回调。

### int uv_accept(uv_stream_t* server, uv_stream_t* client)

此调用与 `uv_listen` 配合使用，用于接受传入连接。在收到 `uv_connection_cb` 后调用此函数来接受连接。调用此函数之前，客户端句柄必须已初始化。返回值 < 0 表示错误。

当 `uv_connection_cb` 回调被调用时，可以保证此函数第一次会成功完成。如果尝试多次使用，则可能失败。建议每次 `uv_connection_cb` 调用只调用此函数一次。

> **注意**：`server` 和 `client` 必须是运行在同一个事件循环上的句柄。

### int uv_read_start(uv_stream_t* stream, uv_alloc_cb alloc_cb, uv_read_cb read_cb)

从传入的流读取数据。`uv_read_cb` 回调会被多次调用，直到没有更多数据可读或调用了 `uv_read_stop`。

*1.38.0 版本变更：`uv_read_start` 现在在被调用两次时一致地返回 `UV_EALREADY`，在流正在关闭时返回 `UV_EINVAL`。在更早的 libuv 版本中，它在 Windows 上返回 `UV_EALREADY` 而在 UNIX 上不返回，在 UNIX 上返回 `UV_EINVAL` 而在 Windows 上不返回。*

### int uv_read_stop(uv_stream_t*)

停止从流读取数据。`uv_read_cb` 回调将不再被调用。

此函数是幂等的，可以安全地对已停止的流调用。

此函数总是成功，因此无需检查其返回值。非零的返回值仅表示在 Windows 上可能还有资源释放工作待该 TTY 的下一个输入事件完成，并不表示失败。

### int uv_write(uv_write_t* req, uv_stream_t* handle, const uv_buf_t bufs[], unsigned int nbufs, uv_write_cb cb)

向流写入数据。缓冲区按顺序写入。示例：

```c
void cb(uv_write_t* req, int status) {
    /* 处理写入结果的逻辑 */
}

uv_buf_t a[] = {
    { .base = "1", .len = 1 },
    { .base = "2", .len = 1 }
};

uv_buf_t b[] = {
    { .base = "3", .len = 1 },
    { .base = "4", .len = 1 }
};

uv_write_t req1;
uv_write_t req2;

/* 写出 "1234" */
uv_write(&req1, stream, a, 2, cb);
uv_write(&req2, stream, b, 2, cb);
```

> **注意**：缓冲区所指向的内存必须在回调被调用之前保持有效。`uv_write2` 同样如此。

### int uv_write2(uv_write_t* req, uv_stream_t* handle, const uv_buf_t bufs[], unsigned int nbufs, uv_stream_t* send_handle, uv_write_cb cb)

用于通过管道发送句柄的扩展写函数。管道必须以 `ipc` == 1 初始化。

> **注意**：在 Unix 上 `send_handle` 必须是 TCP、管道或 UDP 句柄，在 Windows 上必须是 TCP 句柄，且它必须是服务器或已建立的连接（处于监听或已连接状态）。已绑定（bound）的套接字或管道将被视为服务器。

### int uv_try_write(uv_stream_t* handle, const uv_buf_t bufs[], unsigned int nbufs)

与 `uv_write` 相同，但如果写请求无法立即完成，则不会将其加入队列。

将返回以下之一：

- \> 0：已写入的字节数（可能小于提供的缓冲区大小）。
- < 0：负数错误码（如果没有任何数据能被立即发送，返回 `UV_EAGAIN`）。

### int uv_try_write2(uv_stream_t* handle, const uv_buf_t bufs[], unsigned int nbufs, uv_stream_t* send_handle)

与 `uv_try_write` 相同，同时是像 `uv_write2` 那样通过管道发送句柄的扩展写函数。

Windows 不支持尝试发送句柄，此时返回 `UV_EAGAIN`。

*自 1.42.0 版本新增*

### size_t uv_write_nwritten(const uv_write_t* req)

返回写请求已写入的字节数。仅在写回调（`uv_write_cb`）内部调用时有效。

这主要用于写请求通过 `uv_cancel` 被取消、回调收到 `UV_ECANCELED` 状态时，判断取消之前实际写入了多少字节。注意，如果内核在取消生效之前已完成写处理，被取消的写操作仍可能成功或以其他错误失败。

*自 1.53.0 版本新增*

### int uv_is_readable(const uv_stream_t* handle)

如果流可读返回 1，否则返回 0。

### int uv_is_writable(const uv_stream_t* handle)

如果流可写返回 1，否则返回 0。

### int uv_stream_set_blocking(uv_stream_t* handle, int blocking)

启用或禁用流的阻塞模式。

启用阻塞模式后，所有写操作都会同步完成。除此之外接口保持不变，例如操作的完成或失败仍会通过异步触发的回调来报告。

> **警告**：不建议过度依赖此 API。它在将来很可能发生重大变化。
>
> 目前在 Windows 上仅对 `uv_pipe_t` 句柄有效。在 UNIX 平台上，所有 `uv_stream_t` 句柄均受支持。
>
> 此外，libuv 目前在已提交写请求之后才改变阻塞模式时不做任何顺序保证。因此建议在打开或创建流之后立即设置阻塞模式。

*1.4.0 版本变更：新增 UNIX 实现。*

### size_t uv_stream_get_write_queue_size(const uv_stream_t* stream)

返回 `stream->write_queue_size`。

*自 1.19.0 版本新增*

> **参见**：`uv_handle_t` 的 API 函数同样适用。

---

# uv_poll_t — 轮询句柄

轮询（poll）句柄用于监视文件描述符的可读、可写和断开状态，其用途类似于 `poll(2)`。

轮询句柄的用途是集成那些依赖事件循环并需要就套接字状态变化通知它的外部库，比如 c-ares 或 libssh2。不建议将 uv_poll_t 用于任何其他用途；`uv_tcp_t`、`uv_udp_t` 等提供的实现比 `uv_poll_t` 能达到的更快、更具可扩展性，尤其是在 Windows 上。

轮询句柄偶尔会在文件描述符实际上不可读或不可写时仍然发出可读或可写信号。因此用户在尝试对该 fd 进行读或写时，应始终准备好处理 EAGAIN 或等价情况。

同一个套接字上不允许有多个处于活动状态的轮询句柄，这可能导致 libuv 忙循环（busyloop）或出现其他故障。

在被活动中的轮询句柄轮询期间，用户不应关闭文件描述符。这可能导致句柄报告错误，但也可能使它开始轮询另一个套接字。不过，在调用 `uv_poll_stop` 或 `uv_close` 之后可以立即安全地关闭该 fd。

> **注意**：在 Windows 上，只有套接字可以被轮询句柄轮询。在 Unix 上，任何能被 `poll(2)` 接受的文件描述符都可以使用。

> **注意**：在 AIX 上不支持监视断开事件。

## 数据类型

### uv_poll_t

轮询句柄类型。

### void (*uv_poll_cb)(uv_poll_t* handle, int status, int events)

传给 `uv_poll_start` 的回调的类型定义。

### uv_poll_event

轮询事件类型

```c
enum uv_poll_event {
    UV_READABLE = 1,
    UV_WRITABLE = 2,
    UV_DISCONNECT = 4,
    UV_PRIORITIZED = 8
};
```

## 公开成员

无。

> **参见**：`uv_handle_t` 的成员同样适用。

## API

### int uv_poll_init(uv_loop_t* loop, uv_poll_t* handle, int fd)

使用文件描述符初始化句柄。

*1.2.2 版本变更：文件描述符会被设置为非阻塞模式。*

### int uv_poll_init_socket(uv_loop_t* loop, uv_poll_t* handle, uv_os_sock_t socket)

使用套接字描述符初始化句柄。在 Unix 上这与 `uv_poll_init` 相同。在 Windows 上它接受一个 SOCKET 句柄。

*1.2.2 版本变更：套接字会被设置为非阻塞模式。*

### int uv_poll_start(uv_poll_t* handle, int events, uv_poll_cb cb)

开始轮询文件描述符。`events` 是由 `UV_READABLE`、`UV_WRITABLE`、`UV_PRIORITIZED` 和 `UV_DISCONNECT` 组成的位掩码。一旦检测到事件，就会调用回调，此时 `status` 被设为 0，检测到的事件被设置在 `events` 字段上。

`UV_PRIORITIZED` 事件用于监视 sysfs 中断或 TCP 带外（out-of-band）消息。

`UV_DISCONNECT` 事件是可选的，即它可能不会被上报，用户也可以忽略它，但它有助于优化关闭路径，因为可以省去一次额外的读或写调用。

如果在轮询期间发生错误，`status` 将 < 0，并对应某个 `UV_E*` 错误码（参见错误处理一章）。用户不应在句柄活动期间关闭套接字。如果用户还是那样做了，回调*可能*会被调用并报告错误状态，但这**不**被保证。如果 `status == UV_EBADF`，则对该文件句柄的轮询会停止，不再上报任何事件。此时用户应在该句柄上调用 `uv_close`。

> **注意**：对已经处于活动状态的句柄调用 `uv_poll_start` 是可以的。这样做会更新正在被监视的事件掩码。

> **注意**：尽管可以设置 `UV_DISCONNECT`，但它在 AIX 上不受支持，因此在回调的 `events` 字段中不会被设置。

> **注意**：只要设置了 `UV_READABLE` 或 `UV_WRITABLE` 中的某个事件，并且给定的 fd/套接字相应地保持可读或可写，回调就会被再次调用。特别是在以下各种场景中：
>
> - 回调被调用是因为套接字变为可读/可写，而回调根本没有对该套接字进行读/写操作。
> - 回调对套接字执行了读操作，但没有读完所有可用数据（设置了 `UV_READABLE` 时）。
> - 回调对套接字执行了写操作，但套接字之后仍保持可写（设置了 `UV_WRITABLE` 时）。
> - 在对与此套接字关联的轮询句柄调用 `uv_poll_start` 之前，套接字就已经变为可读/可写，且此后套接字的状态没有变化。
>
> 在上述所有场景中，套接字保持可读或可写，因此回调会被再次调用（取决于位掩码中设置的事件）。这种行为称为水平触发（level triggering）。

*1.9.0 版本变更：新增 `UV_DISCONNECT` 事件。*
*1.14.0 版本变更：新增 `UV_PRIORITIZED` 事件。*

### int uv_poll_stop(uv_poll_t* poll)

停止轮询文件描述符，回调将不再被调用。

> **注意**：调用 `uv_poll_stop` 立即生效：任何待处理的回调也会被取消，即使套接字状态变化通知已经处于待处理状态。

> **参见**：`uv_handle_t` 的 API 函数同样适用。

---

# uv_tty_t — TTY 句柄

TTY 句柄表示控制台的一个流。

`uv_tty_t` 是 `uv_stream_t` 的"子类"。

## 数据类型

### uv_tty_t

TTY 句柄类型。

### uv_tty_mode_t

*自 1.2.0 版本新增*

TTY 模式类型：

```c
typedef enum {
    /* 初始/普通终端模式 */
    UV_TTY_MODE_NORMAL,
    /*
    * 原始输入模式（在 Windows 上还会启用 ENABLE_WINDOW_INPUT）。
    * 在未来的 libuv 版本中可能变得等价于 UV_TTY_MODE_RAW_VT。
    */
    UV_TTY_MODE_RAW,
    /* 用于 IPC 的二进制安全 I/O 模式（仅限 Unix） */
    UV_TTY_MODE_IO,
    /* 原始输入模式。在 Windows 上还会设置 ENABLE_VIRTUAL_TERMINAL_INPUT。 */
    UV_TTY_MODE_RAW_VT
} uv_tty_mode_t;
```

### uv_tty_vtermstate_t

控制台虚拟终端模式类型：

```c
typedef enum {
    /*
     * 控制台支持处理虚拟终端序列
     * （Windows 10 新控制台、ConEmu）
     */
    UV_TTY_SUPPORTED,
    /* 控制台无法处理虚拟终端序列。（传统控制台） */
    UV_TTY_UNSUPPORTED
} uv_tty_vtermstate_t
```

## 公开成员

无。

> **参见**：`uv_stream_t` 的成员同样适用。

## API

### int uv_tty_init(uv_loop_t* loop, uv_tty_t* handle, uv_file fd, int unused)

用给定的文件描述符初始化一个新的 TTY 流。文件描述符通常为：

- 0 = stdin
- 1 = stdout
- 2 = stderr

在 Unix 上，此函数会使用 `ttyname_r(3)` 确定 fd 所对应终端的路径并重新打开它，仅当传入的文件描述符确实指向一个 TTY 时才使用它。这使得 libuv 可以把 tty 置于非阻塞模式，而不影响共享该 tty 的其他进程。

在不支持 ioctl TIOCGPTN 或 TIOCPTYGNAME 的系统上（例如 OpenBSD 和 Solaris），此函数不是线程安全的。

> **注意**：如果重新打开 TTY 失败，libuv 会回退到阻塞式写入。

*1.23.1 版本变更：`readable` 参数现已不再使用、被忽略。正确的值现在会由内核自动检测。*

*1.9.0 版本变更：TTY 的路径由 `ttyname_r(3)` 确定。在更早的版本中，libuv 会打开 `/dev/tty`。*

*1.5.0 版本变更：在 UNIX 上，若尝试用一个指向普通文件的文件描述符初始化 TTY 流，则返回 `UV_EINVAL`。*

### int uv_tty_set_mode(uv_tty_t* handle, uv_tty_mode_t mode)

*1.2.0 版本变更：模式现在以 `uv_tty_mode_t` 值指定。*

将 TTY 设置为指定的终端模式。

### int uv_tty_reset_mode(void)

应在程序退出时调用。将 TTY 设置重置为默认值，以交给下一个进程接管。

此函数在 Unix 平台上是异步信号安全的（async signal-safe），但如果在执行处于 `uv_tty_set_mode` 内部时调用它，可能会失败并返回错误码 `UV_EBUSY`。

### int uv_tty_get_winsize(uv_tty_t* handle, int* width, int* height)

获取当前的窗口大小。成功时返回 0。

> **参见**：`uv_stream_t` 的 API 函数同样适用。

### void uv_tty_set_vterm_state(uv_tty_vtermstate_t state)

控制虚拟终端序列由 libuv 还是控制台来处理。对启用 ConEmu 对 ANSI X3.64 和 Xterm 256 色的支持特别有用。否则，Windows 10 控制台通常会被自动检测。

此函数仅在 Windows 系统上有意义。在 Unix 上会被静默忽略。

*自 1.33.0 版本新增*

### int uv_tty_get_vterm_state(uv_tty_vtermstate_t* state)

获取当前虚拟终端序列由 libuv 还是控制台处理的状态。

此函数在 Unix 上未实现，会返回 `UV_ENOTSUP`。

*自 1.33.0 版本新增*
