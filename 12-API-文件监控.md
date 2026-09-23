# 文件监控（FS Event 与 FS Poll）

> 本章译自 libuv 官方文档 api/fs_event.rst 与 api/fs_poll.rst（libuv 1.52）

## FS Event 句柄（`uv_fs_event_t`）

FS Event 句柄允许用户监控给定路径的变更，例如文件被重命名，或文件内容发生了某种变化。该句柄在每个平台上都会使用最适合该任务的底层机制。

> **注意**：对于 AIX，必须安装非默认的 IBM bos.ahafs 软件包。AIX Event Infrastructure 文件系统（ahafs）有一些限制：
>
> - ahafs 按进程跟踪监控，且不是线程安全的。对于同一个事件，必须为每个监控器派生一个单独的进程。
> - 如果只监控所在的文件夹，则不会收到文件修改（写入文件）的事件。
>
> 更多细节请参阅[文档](https://www.ibm.com/docs/en/aix/7.2?topic=management-aix-event-infrastructure-aix-aix-clusters-ahafs)。
>
> z/OS 的文件系统事件监控基础设施不会通知被监控目录内文件的创建/删除。更多细节请参阅 [IBM Knowledge centre](https://www.ibm.com/docs/en/zos/2.2.0?topic=SSLTBW_2.2.0/com.ibm.zos.v2r1.bpxb100/ioc.html)。

### 数据类型

#### `uv_fs_event_t`

FS Event 句柄类型。

#### `void (*uv_fs_event_cb)(uv_fs_event_t* handle, const char* filename, int events, int status)`

传给 `uv_fs_event_start` 的回调，句柄启动后将被反复调用。

如果句柄启动时给定的是一个目录，`filename` 参数将是该目录中所含文件的相对路径；如果无法确定文件名，则为 `NULL`。

`events` 参数是 `uv_fs_event` 枚举元素按位或（OR）得到的掩码。

> **注意**：在 FreeBSD 上，由于一个内核缺陷，`filename` 有时可能为 `NULL`。参见 [FreeBSD bug 参考](https://bugs.freebsd.org/bugzilla/show_bug.cgi?id=197695)。

#### `uv_fs_event`

`uv_fs_event_t` 句柄监控的事件类型。

```c
enum uv_fs_event {
    UV_RENAME = 1,
    UV_CHANGE = 2
};
```

#### `uv_fs_event_flags`

可以传给 `uv_fs_event_start` 以控制其行为的标志。

```c
enum uv_fs_event_flags {
    /*
    * 默认情况下，如果 fs event 监视器被给定一个目录名，我们将监视该目录中的
    * 所有事件。此标志会覆盖这一行为，使 fs_event 只报告目录项本身的变更。
    * 此标志不影响被监视的各个单独文件。
    * 此标志目前在任何后端上均未实现。
    */
    UV_FS_EVENT_WATCH_ENTRY = 1,
    /*
    * 默认情况下 uv_fs_event 会尝试使用内核接口（如 inotify 或 kqueue）来检测
    * 事件。这在远程文件系统（如 NFS 挂载）上可能不可用。此标志使 fs_event 回退
    * 到以固定间隔调用 stat()。
    * 此标志目前在任何后端上均未实现。
    */
    UV_FS_EVENT_STAT = 2,
    /*
    * 默认情况下，事件监视器在监视目录时，不会登记（会忽略）其子目录中的变更。
    * 在支持的平台上，此标志会覆盖这一行为。
    */
    UV_FS_EVENT_RECURSIVE = 4
};
```

### 公开成员

无。

> **参见**：`uv_handle_t` 的成员同样适用。

### API

#### `int uv_fs_event_init(uv_loop_t* loop, uv_fs_event_t* handle)`

```c
int uv_fs_event_init(uv_loop_t* loop, uv_fs_event_t* handle)
```

初始化句柄。

#### `int uv_fs_event_start(uv_fs_event_t* handle, uv_fs_event_cb cb, const char* path, unsigned int flags)`

```c
int uv_fs_event_start(uv_fs_event_t* handle, uv_fs_event_cb cb, const char* path, unsigned int flags)
```

以给定回调启动句柄，该回调将监视指定 `path` 的变更。`flags` 可以是 `uv_fs_event_flags` 枚举元素按位或得到的掩码。

> **注意**：目前唯一支持的标志是 `UV_FS_EVENT_RECURSIVE`，且仅在 macOS（OSX）和 Windows 上可用。
>
> **注意**：在 macOS 上，在调用 `uv_fs_event_start` 之前紧邻时刻由操作系统收集到的事件可能会报告给 `uv_fs_event_cb` 回调。

#### `int uv_fs_event_stop(uv_fs_event_t* handle)`

```c
int uv_fs_event_stop(uv_fs_event_t* handle)
```

停止句柄，回调将不再被调用。

#### `int uv_fs_event_getpath(uv_fs_event_t* handle, char* buffer, size_t* size)`

```c
int uv_fs_event_getpath(uv_fs_event_t* handle, char* buffer, size_t* size)
```

获取句柄正在监控的路径。缓冲区必须由用户预先分配。成功返回 0，失败返回错误码 < 0。成功时，`buffer` 将包含路径，`size` 为其长度。如果缓冲区不够大，将返回 `UV_ENOBUFS`，且 `size` 会被设置为所需大小（含 null 终止符）。

> 1.3.0 变更：返回的长度不再包含结尾的 null 字节，且缓冲区不以 null 结尾。

> 1.9.0 变更：返回 `UV_ENOBUFS` 时返回的长度包含结尾的 null 字节，成功时缓冲区以 null 结尾。

> **参见**：`uv_handle_t` 的 API 函数同样适用。

---

## FS Poll 句柄（`uv_fs_poll_t`）

FS Poll 句柄允许用户监控给定路径的变更。与 `uv_fs_event_t` 不同，fs poll 句柄使用 `stat` 来检测文件是否发生变化，因此可以在 fs event 句柄无法工作的文件系统上工作。

### 数据类型

#### `uv_fs_poll_t`

FS Poll 句柄类型。

#### `void (*uv_fs_poll_cb)(uv_fs_poll_t* handle, int status, const uv_stat_t* prev, const uv_stat_t* curr)`

传给 `uv_fs_poll_start` 的回调，句柄启动后，每当被监控的路径发生任何变化时就会被调用。

如果 `path` 不存在或不可访问，回调将以 `status < 0` 被调用。监视器*不会*停止，但在情况发生变化之前（例如文件被创建，或错误原因发生改变），回调不会再被调用。

当 `status == 0` 时，回调会收到指向新旧 `uv_stat_t` 结构体的指针。它们仅在回调执行期间有效。

### 公开成员

无。

> **参见**：`uv_handle_t` 的成员同样适用。

### API

#### `int uv_fs_poll_init(uv_loop_t* loop, uv_fs_poll_t* handle)`

```c
int uv_fs_poll_init(uv_loop_t* loop, uv_fs_poll_t* handle)
```

初始化句柄。

#### `int uv_fs_poll_start(uv_fs_poll_t* handle, uv_fs_poll_cb poll_cb, const char* path, unsigned int interval)`

```c
int uv_fs_poll_start(uv_fs_poll_t* handle, uv_fs_poll_cb poll_cb, const char* path, unsigned int interval)
```

每隔 `interval` 毫秒检查 `path` 处的文件是否有变化。

> **注意**：为获得最大可移植性，请使用多秒的间隔。亚秒间隔在许多文件系统上无法检测到所有变更。

#### `int uv_fs_poll_stop(uv_fs_poll_t* handle)`

```c
int uv_fs_poll_stop(uv_fs_poll_t* handle)
```

停止句柄，回调将不再被调用。

#### `int uv_fs_poll_getpath(uv_fs_poll_t* handle, char* buffer, size_t* size)`

```c
int uv_fs_poll_getpath(uv_fs_poll_t* handle, char* buffer, size_t* size)
```

获取句柄正在监控的路径。缓冲区必须由用户预先分配。成功返回 0，失败返回错误码 < 0。成功时，`buffer` 将包含路径，`size` 为其长度。如果缓冲区不够大，将返回 `UV_ENOBUFS`，且 `size` 会被设置为所需大小。

> 1.3.0 变更：返回的长度不再包含结尾的 null 字节，且缓冲区不以 null 结尾。

> 1.9.0 变更：返回 `UV_ENOBUFS` 时返回的长度包含结尾的 null 字节，成功时缓冲区以 null 结尾。

> **参见**：`uv_handle_t` 的 API 函数同样适用。
