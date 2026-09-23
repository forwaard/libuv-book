# uv_tcp_t — TCP 句柄

> 本章译自 libuv 官方文档 api/tcp.rst（libuv 1.52）

TCP 句柄用于表示 TCP 流和 TCP 服务器。

`uv_tcp_t` 是 `uv_stream_t` 的"子类"。

## 数据类型

### uv_tcp_t

TCP 句柄类型。

### uv_tcp_flags

`uv_tcp_bind` 中使用的标志位。

```c
enum uv_tcp_flags {
    /* 用于 uv_tcp_bind，当使用 IPv6 地址时。 */
    UV_TCP_IPV6ONLY = 1,

    /* 绑定句柄时启用 SO_REUSEPORT 套接字选项。
     * 如果多个进程或线程在绑定端口之前都设置了 SO_REUSEPORT，
     * 则允许它们进行完全重复的绑定。
     * 传入的连接会分布到参与的各个监听套接字上。
     *
     * 此标志目前仅在 Linux 3.9+、DragonFlyBSD 3.6+、
     * FreeBSD 12.0+、Solaris 11.4 和 AIX 7.2.5+ 上可用。
     */
    UV_TCP_REUSEPORT = 2,
};
```

#### 公开成员

无。

> **参见**：`uv_stream_t` 的成员同样适用。

## API

### int uv_tcp_init(uv_loop_t* loop, uv_tcp_t* handle)

初始化句柄。此时还不会创建套接字。

### int uv_tcp_init_ex(uv_loop_t* loop, uv_tcp_t* handle, unsigned int flags)

以指定的标志初始化句柄。目前 `flags` 参数只有低 8 位被用作套接字域（socket domain）。会为给定的域创建一个套接字。如果指定的域是 `AF_UNSPEC`，则不创建套接字，行为与 `uv_tcp_init` 相同。

*自 1.7.0 版本新增*

### int uv_tcp_open(uv_tcp_t* handle, uv_os_sock_t sock)

将现有的文件描述符或 SOCKET 作为 TCP 句柄打开。

*1.2.1 版本变更：文件描述符会被设置为非阻塞模式。*

> **注意**：传入的文件描述符或 SOCKET 不会做类型检查，但要求它必须代表一个有效的流式套接字。

### int uv_tcp_nodelay(uv_tcp_t* handle, int enable)

启用 `TCP_NODELAY`，即禁用 Nagle 算法。

### int uv_tcp_keepalive(uv_tcp_t* handle, int enable, unsigned int delay)

启用/禁用 TCP keep-alive。`delay` 是以秒为单位的初始延迟，当 `enable` 为零时被忽略。

在 `delay` 时间到达后，还会进行 10 次连续的探测，每次探测间隔 1 秒。如果在此过程结束时连接仍然丢失，则句柄被销毁，并向相应的回调传入 `UV_ETIMEDOUT` 错误。

如果 `delay` 小于 1，则返回 `UV_EINVAL`。

*1.49.0 版本变更：如果 `delay` 小于 1 则返回 `UV_EINVAL`。*

### int uv_tcp_keepalive_ex(uv_tcp_t* handle, int on, unsigned int idle, unsigned int intvl, unsigned int cnt)

启用/禁用 TCP keep-alive，并可设置全部套接字选项：`TCP_KEEPIDLE`、`TCP_KEEPINTVL` 和 `TCP_KEEPCNT`。`idle` 是 `TCP_KEEPIDLE` 的值，`intvl` 是 `TCP_KEEPINTVL` 的值，`cnt` 是 `TCP_KEEPCNT` 的值，当 `on` 为零时被忽略。

启用 TCP keep-alive 后，`idle` 是连接在 TCP 开始发送 keep-alive 探测之前需要保持空闲的时间（秒）。`intvl` 是各次 keep-alive 探测之间的间隔时间（秒）。TCP 在发送 `cnt` 次探测仍未收到对端任何回应后会断开连接，随后句柄被销毁，并向相应的回调传入 `UV_ETIMEDOUT` 错误。

如果 `idle`、`intvl` 或 `cnt` 中任何一个小于 1，则返回 `UV_EINVAL`。

*1.52.0 版本变更：新增对设置 `TCP_KEEPINTVL` 和 `TCP_KEEPCNT` 套接字选项的支持。*

> **注意**：请确保底层操作系统支持这些套接字选项。当前支持的平台：
> - AIX
> - DragonFlyBSD
> - FreeBSD
> - illumos
> - Linux
> - macOS
> - NetBSD
> - Solaris
> - Windows

### int uv_tcp_simultaneous_accepts(uv_tcp_t* handle, int enable)

启用/禁用操作系统在监听新 TCP 连接时排队的同步异步接受请求（simultaneous asynchronous accept requests）。

此设置用于将 TCP 服务器调优到期望的性能。启用同时接受（simultaneous accepts）可以显著提高接受连接的速率（这也是它默认启用的原因），但在多进程部署中可能导致负载分布不均。

### int uv_tcp_bind(uv_tcp_t* handle, const struct sockaddr* addr, unsigned int flags)

将句柄绑定到地址和端口。

当端口已被占用时，你可能会从 `uv_listen` 或 `uv_tcp_connect` 收到 `UV_EADDRINUSE` 错误，除非你为所有绑定的套接字在 `flags` 中指定了 `UV_TCP_REUSEPORT`。也就是说，此函数调用成功并不保证随后对 `uv_listen` 或 `uv_tcp_connect` 的调用也会成功。

**参数**

- `handle`：TCP 句柄。应已通过 `uv_tcp_init` 初始化。
- `addr`：要绑定到的地址。它应指向一个已初始化的 `struct sockaddr_in` 或 `struct sockaddr_in6`。
- `flags`：控制套接字绑定行为的标志。`flags` 中可以包含 `UV_TCP_IPV6ONLY` 以禁用双栈支持，仅使用 IPv6。`flags` 中可以包含 `UV_TCP_REUSEPORT` 以启用 `SO_REUSEPORT` 套接字选项，使其具备负载均衡能力，将传入连接分布到多个进程或线程的所有监听套接字上。

**返回值**：成功返回 0，失败返回小于 0 的错误码。

*1.49.0 版本变更：新增 `UV_TCP_REUSEPORT` 标志。*

> **注意**：`UV_TCP_REUSEPORT` 标志目前仅在 Linux 3.9+、DragonFlyBSD 3.6+、FreeBSD 12.0+、Solaris 11.4 和 AIX 7.2.5+ 上可用。在其他平台上此函数将返回 UV_ENOTSUP 错误。

### int uv_tcp_getsockname(const uv_tcp_t* handle, struct sockaddr* name, int* namelen)

获取句柄当前绑定到的地址。`name` 必须指向一段有效且足够大的内存，推荐使用 `struct sockaddr_storage` 以同时支持 IPv4 和 IPv6。

### int uv_tcp_getpeername(const uv_tcp_t* handle, struct sockaddr* name, int* namelen)

获取连接到该句柄的对端地址。`name` 必须指向一段有效且足够大的内存，推荐使用 `struct sockaddr_storage` 以同时支持 IPv4 和 IPv6。

### int uv_tcp_connect(uv_connect_t* req, uv_tcp_t* handle, const struct sockaddr* addr, uv_connect_cb cb)

建立 IPv4 或 IPv6 的 TCP 连接。需提供一个已初始化的 TCP 句柄和一个未初始化的 `uv_connect_t`。`addr` 应指向一个已初始化的 `struct sockaddr_in` 或 `struct sockaddr_in6`。

在 Windows 上，如果 `addr` 被初始化为指向未指定地址（`0.0.0.0` 或 `::`），它会被改为指向 `localhost`。这样做是为了与 Linux 系统的行为保持一致。

当连接建立完成或发生连接错误时，回调被调用。

*1.19.0 版本变更：新增 `0.0.0.0` 和 `::` 到 `localhost` 的映射。*

> **参见**：`uv_stream_t` 的 API 函数同样适用。

### int uv_tcp_close_reset(uv_tcp_t* handle, uv_close_cb close_cb)

通过发送 RST 包重置 TCP 连接。这是通过将 `SO_LINGER` 套接字选项的 linger 间隔设为 0，然后调用 `uv_close` 来实现的。由于某些平台上的行为不一致，不允许混用 `uv_shutdown` 和 `uv_tcp_close_reset` 调用。

*自 1.32.0 版本新增*

### int uv_socketpair(int type, int protocol, uv_os_sock_t socket_vector[2], int flags0, int flags1)

创建一对具有指定属性的已连接套接字。得到的句柄可以传给 `uv_tcp_open`、用于 `uv_spawn`，或用于其他任何用途。

`flags0` 和 `flags1` 的有效值为：

- UV_NONBLOCK_PIPE：以 `OVERLAPPED` 或 `FIONBIO`/`O_NONBLOCK` I/O 方式打开指定的套接字句柄。对于将被 libuv 使用的句柄，推荐这样做；其他情况通常不建议。

等价于域为 AF_UNIX 的 `socketpair(2)`。

*自 1.41.0 版本新增*

---

# uv_udp_t — UDP 句柄

> 本章译自 libuv 官方文档 api/udp.rst（libuv 1.52）

UDP 句柄为客户端和服务器封装 UDP 通信。

## 数据类型

### uv_udp_t

UDP 句柄类型。

### uv_udp_send_t

UDP 发送请求类型。

### uv_udp_flags

`uv_udp_bind` 和 `uv_udp_recv_cb` 中使用的标志位。

```c
enum uv_udp_flags {
    /* 禁用双栈模式。 */
    UV_UDP_IPV6ONLY = 1,
    /*
     * 表示消息因读取缓冲区太小而被截断。剩余部分已被操作系统丢弃。
     * 在 uv_udp_recv_cb 中使用。
     */
    UV_UDP_PARTIAL = 2,
    /*
     * 表示绑定句柄时是否设置 SO_REUSEADDR。
     * 在 BSD 系统（DragonFlyBSD 除外）、OS X 以及其他 SO_REUSEPORT
     * 不具备负载均衡能力的平台上，这会设置 SO_REUSEPORT 套接字标志，
     * 与 UV_UDP_REUSEPORT 的作用相反。在其他 Unix 平台上，它设置
     * SO_REUSEADDR 标志。这意味着多个线程或进程可以绑定到同一地址
     * 而不出错（前提是它们都设置了该标志），但只有最后绑定的那个会
     * 收到流量，实际上是从之前的监听者那里"偷走"了端口。
     */
    UV_UDP_REUSEADDR = 4,
    /*
     * 表示消息是通过 recvmmsg 接收的，因此提供的缓冲区
     * 不得由 recv_cb 回调释放。
     */
    UV_UDP_MMSG_CHUNK = 8,
    /*
     * 表示提供的缓冲区已被 recvmmsg 完全使用，现在应由 recv_cb
     * 回调释放。当在 uv_udp_recv_cb 中设置此标志时，nread 总是 0，
     * addr 总是 NULL。
     */
    UV_UDP_MMSG_FREE = 16,
    /*
     * 表示绑定句柄时是否设置 IP_RECVERR/IPV6_RECVERR。
     * 在 Linux 上，这会为 IPv4 UDP 套接字设置 IP_RECVERR，为 IPv6 UDP
     * 套接字设置 IPV6_RECVERR。这可以阻止 Linux 内核抑制某些 ICMP 错误
     * 消息，并启用完整的 ICMP 错误报告以加快故障转移。
     * 此标志在 Linux 以外的平台上是空操作。
     */
    UV_UDP_LINUX_RECVERR = 32,
    /*
     * 表示绑定句柄时是否设置 SO_REUSEPORT。
     * 这会在支持的平台上设置 SO_REUSEPORT 套接字选项。
     * 与 UV_UDP_REUSEADDR 不同，此标志会让绑定到相同地址和端口的
     * 多个线程或进程"共享"该端口，也就是说传入的数据报会分布到
     * 各线程或进程的接收套接字上。
     *
     * 此标志目前仅在 Linux 3.9+、DragonFlyBSD 3.6+、
     * FreeBSD 12.0+、Solaris 11.4 和 AIX 7.2.5+ 上可用。
     */
    UV_UDP_REUSEPORT = 64,
    /*
     * 表示应使用 recvmmsg（如果可用）。此句柄的 uv_alloc_cb
     * 应创建以 64 KiB 为倍数大小的缓冲区。
     */
    UV_UDP_RECVMMSG = 256
};
```

### void (*uv_udp_send_cb)(uv_udp_send_t* req, int status)

传给 `uv_udp_send` 的回调的类型定义，在数据发送完成后被调用。

### void (*uv_udp_recv_cb)(uv_udp_t* handle, ssize_t nread, const uv_buf_t* buf, const struct sockaddr* addr, unsigned flags)

传给 `uv_udp_recv_start` 的回调的类型定义，当端点收到数据时被调用。

- `handle`：UDP 句柄
- `nread`：已接收的字节数。如果没有更多数据可读则为 0。注意 0 也可能表示收到了一个空数据报（此时 `addr` 不为 NULL）。如果检测到传输错误则 < 0；如果使用 `recvmmsg(2)`，则不会再收到更多分块，缓冲区可以被安全释放。
- `buf`：包含已接收数据的 `uv_buf_t`。
- `addr`：包含发送方地址的 `struct sockaddr*`。可以为 NULL。仅在回调执行期间有效。
- `flags`：一个或多个按位或组合的 UV_UDP_* 常量。

被调用方负责释放缓冲区，libuv 不会复用它。出错时缓冲区可能是一个空缓冲区（即 `buf->base` == NULL 且 `buf->len` == 0）。

使用 `recvmmsg(2)` 时，分块会带有 `UV_UDP_MMSG_CHUNK` 标志，这些分块不得被释放。如果没有发生错误，最后会以 `nread` 为 0、`addr` 为 NULL 的形式再调用一次回调，缓冲区指向最初分配的数据，`UV_UDP_MMSG_CHUNK` 标志被清除而 `UV_UDP_MMSG_FREE` 标志被设置。如果发生 UDP 套接字错误，`nread` 将 < 0。无论哪种情况，被调用方此时都可以安全地释放提供的缓冲区。

*1.40.0 版本变更：新增 `UV_UDP_MMSG_FREE` 标志。*

> **注意**：当没有内容可读时，接收回调会以 `nread` == 0 且 `addr` == NULL 被调用；当收到一个空的 UDP 包时，则以 `nread` == 0 且 `addr` != NULL 被调用。

### uv_membership

多播地址的成员关系类型。

```c
typedef enum {
    UV_LEAVE_GROUP = 0,
    UV_JOIN_GROUP
} uv_membership;
```

## 公开成员

### size_t uv_udp_t.send_queue_size

已排队等待发送的字节数。此字段严格反映当前排队的信息量。

### size_t uv_udp_t.send_queue_count

当前在队列中等待处理的发送请求数量。

### uv_udp_t* uv_udp_send_t.handle

此发送请求所在的 UDP 句柄。

> **参见**：`uv_handle_t` 的成员同样适用。

## API

### int uv_udp_init(uv_loop_t* loop, uv_udp_t* handle)

初始化一个新的 UDP 句柄。实际的套接字是惰性创建的。成功返回 0。

### int uv_udp_init_ex(uv_loop_t* loop, uv_udp_t* handle, unsigned int flags)

以指定的标志初始化句柄。`flags` 参数的低 8 位用作套接字域。会为给定的域创建套接字。如果指定的域是 `AF_UNSPEC`，则不创建套接字，行为与 `uv_udp_init` 相同。

其余位可用于设置以下标志之一：

- `UV_UDP_RECVMMSG`：如果设置且平台支持，将使用 `recvmmsg(2)`。此句柄的 `uv_alloc_cb` 应创建以 64 KiB 为倍数大小的缓冲区。

*自 1.7.0 版本新增*
*1.37.0 版本变更：新增 `UV_UDP_RECVMMSG` 标志。*

### int uv_udp_open_ex(uv_udp_t* handle, uv_os_sock_t sock, unsigned int flags)

将现有的文件描述符或 Windows SOCKET 作为 UDP 句柄打开。

**参数**

- `handle`：UDP 句柄。应已通过 `uv_udp_init` 初始化。
- `sock`：要与句柄关联的现有套接字。
- `flags`：控制套接字行为的标志，支持 `UV_UDP_REUSEADDR` 和 `UV_UDP_REUSEPORT`。

**返回值**：成功返回 0，失败返回小于 0 的错误码。

*自 1.52.0 版本新增*

### int uv_udp_open(uv_udp_t* handle, uv_os_sock_t sock)

将现有的文件描述符或 Windows SOCKET 作为 UDP 句柄打开。

仅限 Unix：对 `sock` 参数的唯一要求是它遵循数据报契约（可在未连接模式下工作、支持 sendmsg()/recvmsg() 等）。换句话说，其他数据报类型的套接字如原始套接字（raw socket）或 netlink 套接字也可以传给此函数。

*1.2.1 版本变更：文件描述符会被设置为非阻塞模式。*

> **注意**：传入的文件描述符或 SOCKET 不会做类型检查，但要求它必须代表一个有效的数据报套接字。
>
> 内部会无条件设置 SO_REUSEADDR 套接字选项。这意味着复用标志总是被启用，无论用户意图如何。如需更多控制，请使用 `uv_udp_open_ex`。

### int uv_udp_bind(uv_udp_t* handle, const struct sockaddr* addr, unsigned int flags)

将 UDP 句柄绑定到 IP 地址和端口。

**参数**

- `handle`：UDP 句柄。应已通过 `uv_udp_init` 初始化。
- `addr`：包含要绑定的地址和端口的 `struct sockaddr_in` 或 `struct sockaddr_in6`。
- `flags`：指示套接字的绑定方式，支持 `UV_UDP_IPV6ONLY`、`UV_UDP_REUSEADDR`、`UV_UDP_REUSEPORT` 和 `UV_UDP_RECVERR`。

**返回值**：成功返回 0，失败返回小于 0 的错误码。

*1.49.0 版本变更：新增 `UV_UDP_REUSEPORT` 标志。*

> **注意**：`UV_UDP_REUSEPORT` 标志目前仅在 Linux 3.9+、DragonFlyBSD 3.6+、FreeBSD 12.0+、Solaris 11.4 和 AIX 7.2.5+ 上可用。在其他平台上此函数将返回 UV_ENOTSUP 错误。
> 对于 SO_REUSEPORT 具备负载均衡能力的平台，允许在 flags 中同时指定 `UV_UDP_REUSEADDR` 和 `UV_UDP_REUSEPORT`，且 `SO_REUSEPORT` 总是覆盖 `SO_REUSEADDR` 的行为。
> 对于 SO_REUSEPORT 不具备负载均衡能力的平台，在 flags 中同时指定 `UV_UDP_REUSEADDR` 和 `UV_UDP_REUSEPORT` 会失败，返回 UV_ENOTSUP 错误。

### int uv_udp_connect(uv_udp_t* handle, const struct sockaddr* addr)

将 UDP 句柄关联到远程地址和端口，此后该句柄发出的每条消息都会自动发送到该目的地。以 `NULL` 的 `addr` 调用此函数会断开句柄的连接。对已连接的句柄再次调用 `uv_udp_connect()` 会导致 `UV_EISCONN` 错误。尝试断开一个未连接的句柄会返回 `UV_ENOTCONN` 错误。

**参数**

- `handle`：UDP 句柄。应已通过 `uv_udp_init` 初始化。
- `addr`：包含要关联的地址和端口的 `struct sockaddr_in` 或 `struct sockaddr_in6`。

**返回值**：成功返回 0，失败返回小于 0 的错误码。

*自 1.27.0 版本新增*

### int uv_udp_getpeername(const uv_udp_t* handle, struct sockaddr* name, int* namelen)

在已连接的 UDP 句柄上获取其远程 IP 和端口。在未连接的句柄上，返回 `UV_ENOTCONN`。

**参数**

- `handle`：UDP 句柄。应已通过 `uv_udp_init` 初始化并绑定。
- `name`：指向将被填入地址数据的结构的指针。为支持 IPv4 和 IPv6，应使用 `struct sockaddr_storage`。
- `namelen`：输入时表示 `name` 字段的容量；输出时表示实际填充了多少。

**返回值**：成功返回 0，失败返回小于 0 的错误码。

*自 1.27.0 版本新增*

### int uv_udp_getsockname(const uv_udp_t* handle, struct sockaddr* name, int* namelen)

获取 UDP 句柄的本地 IP 和端口。

**参数**

- `handle`：UDP 句柄。应已通过 `uv_udp_init` 初始化并绑定。
- `name`：指向将被填入地址数据的结构的指针。为支持 IPv4 和 IPv6，应使用 `struct sockaddr_storage`。
- `namelen`：输入时表示 `name` 字段的容量；输出时表示实际填充了多少。

**返回值**：成功返回 0，失败返回小于 0 的错误码。

### int uv_udp_set_membership(uv_udp_t* handle, const char* multicast_addr, const char* interface_addr, uv_membership membership)

设置多播地址的成员关系。

**参数**

- `handle`：UDP 句柄。应已通过 `uv_udp_init` 初始化。
- `multicast_addr`：要设置成员关系的多播地址。
- `interface_addr`：接口地址。
- `membership`：应为 `UV_JOIN_GROUP` 或 `UV_LEAVE_GROUP`。

**返回值**：成功返回 0，失败返回小于 0 的错误码。

### int uv_udp_set_source_membership(uv_udp_t* handle, const char* multicast_addr, const char* interface_addr, const char* source_addr, uv_membership membership)

设置特定源多播组（source-specific multicast group）的成员关系。

**参数**

- `handle`：UDP 句柄。应已通过 `uv_udp_init` 初始化。
- `multicast_addr`：要设置成员关系的多播地址。
- `interface_addr`：接口地址。
- `source_addr`：源地址。
- `membership`：应为 `UV_JOIN_GROUP` 或 `UV_LEAVE_GROUP`。

**返回值**：成功返回 0，失败返回小于 0 的错误码。

*自 1.32.0 版本新增*

### int uv_udp_set_multicast_loop(uv_udp_t* handle, int on)

设置 IP 多播回环标志。使多播数据包回环到本地套接字。

**参数**

- `handle`：UDP 句柄。应已通过 `uv_udp_init_ex` 以 `AF_INET` 或 `AF_INET6` 初始化，或已通过 `uv_udp_bind` 显式绑定到地址，或已通过 `uv_udp_send` 或 `uv_udp_recv_start` 隐式绑定。
- `on`：1 表示开启，0 表示关闭。

**返回值**：成功返回 0，失败返回小于 0 的错误码。

### int uv_udp_set_multicast_ttl(uv_udp_t* handle, int ttl)

设置多播 TTL。

**参数**

- `handle`：UDP 句柄。应已通过 `uv_udp_init_ex` 以 `AF_INET` 或 `AF_INET6` 初始化，或已通过 `uv_udp_bind` 显式绑定到地址，或已通过 `uv_udp_send` 或 `uv_udp_recv_start` 隐式绑定。
- `ttl`：1 到 255。

**返回值**：成功返回 0，失败返回小于 0 的错误码。

### int uv_udp_set_multicast_interface(uv_udp_t* handle, const char* interface_addr)

设置用于发送或接收数据的多播接口。

**参数**

- `handle`：UDP 句柄。应已通过 `uv_udp_init_ex` 以 `AF_INET` 或 `AF_INET6` 初始化，或已通过 `uv_udp_bind` 显式绑定到地址，或已通过 `uv_udp_send` 或 `uv_udp_recv_start` 隐式绑定。
- `interface_addr`：接口地址。

**返回值**：成功返回 0，失败返回小于 0 的错误码。

### int uv_udp_set_broadcast(uv_udp_t* handle, int on)

开启或关闭广播。

**参数**

- `handle`：UDP 句柄。应已通过 `uv_udp_init_ex` 以 `AF_INET` 或 `AF_INET6` 初始化，或已通过 `uv_udp_bind` 显式绑定到地址，或已通过 `uv_udp_send` 或 `uv_udp_recv_start` 隐式绑定。
- `on`：1 表示开启，0 表示关闭。

**返回值**：成功返回 0，失败返回小于 0 的错误码。

### int uv_udp_set_ttl(uv_udp_t* handle, int ttl)

设置生存时间（time to live）。

**参数**

- `handle`：UDP 句柄。应已通过 `uv_udp_init_ex` 以 `AF_INET` 或 `AF_INET6` 初始化，或已通过 `uv_udp_bind` 显式绑定到地址，或已通过 `uv_udp_send` 或 `uv_udp_recv_start` 隐式绑定。
- `ttl`：1 到 255。

**返回值**：成功返回 0，失败返回小于 0 的错误码。

### int uv_udp_send(uv_udp_send_t* req, uv_udp_t* handle, const uv_buf_t bufs[], unsigned int nbufs, const struct sockaddr* addr, uv_udp_send_cb send_cb)

通过 UDP 套接字发送数据。如果套接字之前未通过 `uv_udp_bind` 绑定，它将被绑定到 0.0.0.0（即"所有接口"的 IPv4 地址）和一个随机端口。

在 Windows 上，如果 `addr` 被初始化为指向未指定地址（`0.0.0.0` 或 `::`），它会被改为指向 `localhost`。这样做是为了与 Linux 系统的行为保持一致。

对于已连接的 UDP 句柄，`addr` 必须设为 `NULL`，否则将返回 `UV_EISCONN` 错误。

对于未连接的 UDP 句柄，`addr` 不能为 `NULL`，否则将返回 `UV_EDESTADDRREQ` 错误。

**参数**

- `req`：UDP 请求句柄。无需初始化。
- `handle`：UDP 句柄。应已通过 `uv_udp_init` 初始化。
- `bufs`：要发送的缓冲区列表。
- `nbufs`：`bufs` 中缓冲区的数量。
- `addr`：包含远程对端地址和端口的 `struct sockaddr_in` 或 `struct sockaddr_in6`。
- `send_cb`：数据发出后要调用的回调。

**返回值**：成功返回 0，失败返回小于 0 的错误码。

*1.19.0 版本变更：新增 `0.0.0.0` 和 `::` 到 `localhost` 的映射。*

*1.27.0 版本变更：新增对已连接套接字的支持。*

### int uv_udp_try_send(uv_udp_t* handle, const uv_buf_t bufs[], unsigned int nbufs, const struct sockaddr* addr)

与 `uv_udp_send` 相同，但如果发送请求无法立即完成，则不会将其加入队列。

对于已连接的 UDP 句柄，`addr` 必须设为 `NULL`，否则将返回 `UV_EISCONN` 错误。

对于未连接的 UDP 句柄，`addr` 不能为 `NULL`，否则将返回 `UV_EDESTADDRREQ` 错误。

**返回值**：>= 0：已发送的字节数（与给定的缓冲区大小一致）。< 0：负数错误码（当消息无法立即发送时返回 `UV_EAGAIN`）。

*1.27.0 版本变更：新增对已连接套接字的支持。*

### int uv_udp_try_send2(uv_udp_t* handle, unsigned int count, uv_buf_t* bufs[/*count*/], unsigned int nbufs[/*count*/], struct sockaddr* addrs[/*count*/], unsigned int flags)

与 `uv_udp_try_send` 类似，但可以一次发送多个数据报。是对 `sendmmsg(2)` 的轻量抽象，对不支持该系统调用的平台使用 `sendmsg(2)` 回退循环。句柄必须已完全初始化；请先调用 `uv_udp_bind`。

**返回值**：>= 0：已发送的数据报数。仅当 `count` 为零时才为 0。< 0：负数错误码。仅当发送第一个数据报失败时才返回负数，否则返回正的发送数量。当数据报当前无法发送时返回 `UV_EAGAIN`；此时应回退到 `uv_udp_send`。

*自 1.50.0 版本新增*

### int uv_udp_recv_start(uv_udp_t* handle, uv_alloc_cb alloc_cb, uv_udp_recv_cb recv_cb)

准备接收数据。如果套接字之前未通过 `uv_udp_bind` 绑定，它会被绑定到 0.0.0.0（即"所有接口"的 IPv4 地址）和一个随机端口。

**参数**

- `handle`：UDP 句柄。应已通过 `uv_udp_init` 初始化。
- `alloc_cb`：需要临时存储时调用的回调。
- `recv_cb`：收到数据时调用的回调。

**返回值**：成功返回 0，失败返回小于 0 的错误码。

> **注意**：使用 `recvmmsg(2)` 时，一次接收的消息数量受限于能在 `alloc_cb` 分配的缓冲区中容纳的最大尺寸数据报（dgram）的数量，且 udp_recv 的 `alloc_cb` 中的 `suggested_size` 总是被设置为 1 个最大尺寸数据报的大小。

*1.35.0 版本变更：在支持的平台上新增对 `recvmmsg(2)` 的支持。此句柄的 `uv_alloc_cb` 应创建以 64 KiB 为倍数大小的缓冲区。*
*1.37.0 版本变更：`recvmmsg(2)` 支持不再被隐式启用，必须通过向 `uv_udp_init_ex` 传入 `UV_UDP_RECVMMSG` 标志显式请求。*
*1.39.0 版本变更：`uv_udp_using_recvmmsg` 可在 `alloc_cb` 中用于判断是否应为当前句柄/平台分配一个适用于 `recvmmsg(2)` 大小的缓冲区。*

### int uv_udp_using_recvmmsg(uv_udp_t* handle)

如果 UDP 句柄是以 `UV_UDP_RECVMMSG` 标志创建且平台支持 `recvmmsg(2)`，则返回 1；否则返回 0。

*自 1.39.0 版本新增*

### int uv_udp_recv_stop(uv_udp_t* handle)

停止监听传入的数据报。

**参数**

- `handle`：UDP 句柄。应已通过 `uv_udp_init` 初始化。

**返回值**：成功返回 0，失败返回小于 0 的错误码。

### size_t uv_udp_get_send_queue_size(const uv_udp_t* handle)

返回 `handle->send_queue_size`。

*自 1.19.0 版本新增*

### size_t uv_udp_get_send_queue_count(const uv_udp_t* handle)

返回 `handle->send_queue_count`。

*自 1.19.0 版本新增*

> **参见**：`uv_handle_t` 的 API 函数同样适用。
