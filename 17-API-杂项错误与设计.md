# 杂项工具 · 错误处理 · 设计概述 · 共享库处理

> 本章译自 libuv 官方文档 api/misc.rst、api/errors.rst、api/design.rst、api/dll.rst（libuv 1.52）

---

# 杂项工具

本节包含不适合归入其他任何小节的杂项函数。

## 数据类型

### uv_buf_t

缓冲区数据类型。

#### char* uv_buf_t.base

指向缓冲区基地址的指针。

#### size_t uv_buf_t.len

缓冲区中的总字节数。

> **注意**：在 Windows 上此字段为 ULONG。

### void* (*uv_malloc_func)(size_t size)

`malloc(3)` 的替换函数。参见 `uv_replace_allocator`。

### void* (*uv_realloc_func)(void* ptr, size_t size)

`realloc(3)` 的替换函数。参见 `uv_replace_allocator`。

### void* (*uv_calloc_func)(size_t count, size_t size)

`calloc(3)` 的替换函数。参见 `uv_replace_allocator`。

### void (*uv_free_func)(void* ptr)

`free(3)` 的替换函数。参见 `uv_replace_allocator`。

### void (*uv_random_cb)(uv_random_t* req, int status, void* buf, size_t buflen)

传给 `uv_random` 的回调。出错时 `status` 非零。`buf` 指针与传给 `uv_random` 的指针相同。

### uv_file

文件句柄的跨平台表示。

### uv_os_sock_t

套接字句柄的跨平台表示。

### uv_os_fd_t

文件描述符的抽象表示。在 Unix 系统上它是 `int` 的 `typedef`，在 Windows 上是 `HANDLE`。

### uv_pid_t

`pid_t` 的跨平台表示。

*自 1.16.0 版本新增*

### uv_timeval_t

以微秒分辨率存储时间的非 Y2K38 安全数据类型。将在 libuv v2.0 中被 `uv_timeval64_t` 取代。

```c
typedef struct {
    long tv_sec;
    long tv_usec;
} uv_timeval_t;
```

### uv_timeval64_t

以微秒分辨率存储时间的 Y2K38 安全数据类型。

```c
typedef struct {
    int64_t tv_sec;
    int32_t tv_usec;
} uv_timeval64_t;
```

### uv_timespec64_t

以纳秒分辨率存储时间的 Y2K38 安全数据类型。

```c
typedef struct {
    int64_t tv_sec;
    int32_t tv_nsec;
} uv_timespec64_t;
```

### uv_clock_id

`uv_clock_gettime` 使用的时钟源。

```c
typedef enum {
  UV_CLOCK_MONOTONIC,
  UV_CLOCK_REALTIME
} uv_clock_id;
```

### uv_rusage_t

资源占用结果的数据类型。

```c
typedef struct {
    uv_timeval_t ru_utime; /* 使用的用户 CPU 时间 */
    uv_timeval_t ru_stime; /* 使用的系统 CPU 时间 */
    uint64_t ru_maxrss; /* 最大常驻内存集大小 */
    uint64_t ru_ixrss; /* 共享内存总量 (X) */
    uint64_t ru_idrss; /* 非共享数据总量 (X) */
    uint64_t ru_isrss; /* 非共享栈总量 (X) */
    uint64_t ru_minflt; /* 页回收（软缺页） (X) */
    uint64_t ru_majflt; /* 缺页（硬缺页） */
    uint64_t ru_nswap; /* 换出次数 (X) */
    uint64_t ru_inblock; /* 块输入操作 */
    uint64_t ru_oublock; /* 块输出操作 */
    uint64_t ru_msgsnd; /* 发送的 IPC 消息 (X) */
    uint64_t ru_msgrcv; /* 接收的 IPC 消息 (X) */
    uint64_t ru_nsignals; /* 收到的信号 (X) */
    uint64_t ru_nvcsw; /* 自愿上下文切换 (X) */
    uint64_t ru_nivcsw; /* 非自愿上下文切换 (X) */
} uv_rusage_t;
```

标记 `(X)` 的成员在 Windows 上不受支持。类 UNIX 平台上支持的字段参见 `getrusage(2)`。

最大常驻内存集大小以千字节（KB）为单位报告，这是大多数平台原生使用的单位。

### uv_cpu_info_t

CPU 信息的数据类型。

```c
typedef struct uv_cpu_info_s {
    char* model;
    int speed;
    struct uv_cpu_times_s {
        uint64_t user; /* 毫秒 */
        uint64_t nice; /* 毫秒 */
        uint64_t sys; /* 毫秒 */
        uint64_t idle; /* 毫秒 */
        uint64_t irq; /* 毫秒 */
    } cpu_times;
} uv_cpu_info_t;
```

### uv_interface_address_t

接口地址的数据类型。

```c
typedef struct uv_interface_address_s {
    char* name;
    char phys_addr[6];
    int is_internal;
    union {
        struct sockaddr_in address4;
        struct sockaddr_in6 address6;
    } address;
    union {
        struct sockaddr_in netmask4;
        struct sockaddr_in6 netmask6;
    } netmask;
} uv_interface_address_t;
```

### uv_passwd_t

密码文件（password file）信息的数据类型。

```c
typedef struct uv_passwd_s {
    char* username;
    long uid;
    long gid;
    char* shell;
    char* homedir;
} uv_passwd_t;
```

### uv_group_t

组文件（group file）信息的数据类型。

```c
typedef struct uv_group_s {
  char* groupname;
  unsigned long gid;
  char** members;
} uv_group_t;
```

### uv_utsname_t

操作系统名称和版本信息的数据类型。

```c
typedef struct uv_utsname_s {
    char sysname[256];
    char release[256];
    char version[256];
    char machine[256];
} uv_utsname_t;
```

### uv_env_item_t

环境变量存储的数据类型。

```c
typedef struct uv_env_item_s {
    char* name;
    char* value;
} uv_env_item_t;
```

### uv_random_t

随机数据请求类型。

## API

### uv_handle_type uv_guess_handle(uv_file file)

用于检测给定的文件描述符应使用哪种类型的流。通常会在初始化阶段用它来猜测 stdio 流的类型。

要获得 `isatty(3)` 等价的功能，可以使用此函数并检测是否为 `UV_TTY`。

### int uv_replace_allocator(uv_malloc_func malloc_func, uv_realloc_func realloc_func, uv_calloc_func calloc_func, uv_free_func free_func)

*自 1.6.0 版本新增*

覆盖标准库的 `malloc(3)`、`calloc(3)`、`realloc(3)`、`free(3)` 内存分配函数。

此函数必须在任何其他 libuv 函数被调用之前调用，或者在所有资源已被释放、libuv 不再引用任何已分配内存块之后调用。

成功时返回 0；如果任何一个函数指针为 `NULL`，返回 `UV_EINVAL`。

> **警告**：没有针对多次更换分配器的保护。如果用户更换了分配器，他们有责任确保更换时没有用旧分配器分配过内存，或新旧分配器是兼容的。

> **警告**：分配器必须是线程安全的。

### void uv_library_shutdown(void);

*自 1.38.0 版本新增*

释放 libuv 持有的所有全局状态。libuv 通常会在自身被卸载时自动完成这一操作，但也可以指示它手动执行清理。

> **警告**：`uv_library_shutdown` 只能调用一次。

> **警告**：当仍有事件循环或 I/O 请求处于活动状态时，不要调用 `uv_library_shutdown`。

> **警告**：调用 `uv_library_shutdown` 之后不要再调用任何 libuv 函数。

### uv_buf_t uv_buf_init(char* base, unsigned int len)

`uv_buf_t` 的构造函数。

由于平台差异，用户不能依赖 uv_buf_t 结构体中 `base` 和 `len` 成员的顺序。uv_buf_t 用完后由用户负责释放 `base`。结构体按值返回。

> **警告**：不建议把 `len` 设置为很大的值，那可能导致虚假的失败。具体来说，Windows 上超过约 511 MB 的写入可能失败，各种类 Unix 系统上超过约 2 GB（0x7ffff000 字节）的 I/O 可能失败。通常更好的做法是把数据拆分成多次 `uv_write` 调用（把 `write_cb` 附加到最后一次调用上）。

### char** uv_setup_args(int argc, char** argv)

保存程序参数。获取/设置进程标题或可执行文件路径时需要它。libuv 可能会接管 `argv` 所指向的内存。此函数应在程序启动时恰好调用一次。

示例：

```c
argv = uv_setup_args(argc, argv);  /* 可能返回 argv 的副本。 */
```

### int uv_get_process_title(char* buffer, size_t size)

获取当前进程的标题。在 Unix 和 AIX 系统上调用此函数之前你*必须*先调用 `uv_setup_args`。如果在需要它的系统上尚未调用过 `uv_setup_args`，则返回 `UV_ENOBUFS`。如果 `buffer` 为 `NULL` 或 `size` 为零，返回 `UV_EINVAL`。如果 `size` 无法容纳进程标题和结尾的 `nul` 字符，函数返回 `UV_ENOBUFS`。

> **注意**：在 BSD 系统上，获取初始进程标题需要 `uv_setup_args`。在调用 `uv_setup_args` 或 `uv_set_process_title` 之前，返回的进程标题将是空字符串。

*1.18.1 版本变更：现在在所有支持的平台上都是线程安全的。*

*1.39.0 版本变更：现在如果需要 `uv_setup_args` 但尚未调用，会返回错误。*

### int uv_set_process_title(const char* title)

设置当前进程的标题。在 Unix 和 AIX 系统上调用此函数之前你*必须*先调用 `uv_setup_args`。如果在需要它的系统上尚未调用过 `uv_setup_args`，则返回 `UV_ENOBUFS`。在为进程标题提供固定大小缓冲区的平台上，`title` 的内容会被复制到缓冲区中，如果超出可用空间则被截断。其他平台如果无法分配足够的空间来复制 `title` 的内容，将返回 `UV_ENOMEM`。

*1.18.1 版本变更：现在在所有支持的平台上都是线程安全的。*

*1.39.0 版本变更：现在如果需要 `uv_setup_args` 但尚未调用，会返回错误。*

### int uv_resident_set_memory(size_t* rss)

获取当前进程的常驻内存集大小（RSS）。

### int uv_uptime(double* uptime)

获取当前系统的运行时间（uptime）。根据系统的不同，返回完整的或带小数的秒数。

### int uv_getrusage(uv_rusage_t* rusage)

获取当前进程的资源占用度量。

> **注意**：在 Windows 上并非所有字段都会被设置，不支持的字段以零填充。更多细节参见 `uv_rusage_t`。

### int uv_getrusage_thread(uv_rusage_t* rusage)

获取调用线程的资源占用度量。

*自 1.50.0 版本新增*

> **注意**：并非所有平台都支持。可能返回 `UV_ENOTSUP`。在 macOS 和 Windows 上并非所有字段都会被设置，不支持的字段以零填充。更多细节参见 `uv_rusage_t`。

### uv_pid_t uv_os_getpid(void)

返回当前进程的进程 ID。

*自 1.18.0 版本新增*

### uv_pid_t uv_os_getppid(void)

返回父进程的进程 ID。

*自 1.16.0 版本新增*

### unsigned int uv_available_parallelism(void)

返回程序应使用的默认并行度估计值。总是返回非零值。

在 Linux 上，会检查调用线程的 CPU 亲和性掩码，以判断它是否被固定到了特定的 CPU。

在 Windows 上，当系统拥有超过 64 个逻辑 CPU 时，可用并行度可能被低估。

在其他平台上，报告操作系统认为处于在线状态的 CPU 数量。

*自 1.44.0 版本新增*

### int uv_cpu_info(uv_cpu_info_t** cpu_infos, int* count)

获取系统上 CPU 的信息。`cpu_infos` 数组将包含 `count` 个元素，需要用 `uv_free_cpu_info` 释放。

如果你需要知道有多少 CPU 可供线程或子进程使用，请使用 `uv_available_parallelism`。

### void uv_free_cpu_info(uv_cpu_info_t* cpu_infos, int count)

释放之前由 `uv_cpu_info` 分配的 `cpu_infos` 数组。

### int uv_cpumask_size(void)

返回用于进程/线程亲和性的掩码的最大大小；如果当前平台不支持亲和性，则返回 `UV_ENOTSUP`。

*自 1.45.0 版本新增*

### int uv_interface_addresses(uv_interface_address_t** addresses, int* count)

获取系统上网络接口的地址信息。会分配一个包含 `count` 个元素的数组并在 `addresses` 中返回。用户必须调用 `uv_free_interface_addresses` 释放它。

### void uv_free_interface_addresses(uv_interface_address_t* addresses, int count)

释放由 `uv_interface_addresses` 返回的 `uv_interface_address_t` 数组。

### void uv_loadavg(double avg[3])

获取平均负载。参见：[Load (computing) - Wikipedia](https://en.wikipedia.org/wiki/Load_(computing))

> **注意**：在 Windows 上返回 [0,0,0]（即未实现）。

### int uv_ip4_addr(const char* ip, int port, struct sockaddr_in* addr)

将包含 IPv4 地址的字符串转换为二进制结构。

### int uv_ip6_addr(const char* ip, int port, struct sockaddr_in6* addr)

将包含 IPv6 地址的字符串转换为二进制结构。

### int uv_ip4_name(const struct sockaddr_in* src, char* dst, size_t size)

将包含 IPv4 地址的二进制结构转换为字符串。

### int uv_ip6_name(const struct sockaddr_in6* src, char* dst, size_t size)

将包含 IPv6 地址的二进制结构转换为字符串。

### int uv_ip_name(const struct sockaddr *src, char *dst, size_t size)

将包含 IPv4 地址或 IPv6 地址的二进制结构转换为字符串。

### int uv_inet_ntop(int af, const void* src, char* dst, size_t size)
### int uv_inet_pton(int af, const char* src, void* dst)

支持 IPv6 的 `inet_ntop(3)` 和 `inet_pton(3)` 跨平台实现。成功时返回 0。出错时目标 `dst` 指针保持不变。

### UV_IF_NAMESIZE

IPv6 接口标识符名称的最大长度。在 Unix 上定义为 `IFNAMSIZ`，在 Linux 和 Windows 上定义为 `IF_NAMESIZE`。

*自 1.16.0 版本新增*

### int uv_if_indextoname(unsigned int ifindex, char* buffer, size_t* size)

支持 IPv6 的 `if_indextoname(3)` 实现。调用时 `*size` 表示 `buffer` 的长度，用于存放结果。成功时返回零，`buffer` 包含接口名，`*size` 表示 `buffer` 的字符串长度（不含 NUL 结尾字节，NUL 结尾字节不计入 `*size`）。出错时返回负数。如果 `buffer` 不足以容纳结果，返回 `UV_ENOBUFS`，且 `*size` 表示所需的字节大小（包含 NUL 结尾字节，计入 `*size`）。

在 Unix 上，返回的接口名可以直接用作带作用域的 IPv6 地址中的接口标识符，例如 `fe80::abc:def1:2345%en0`。

在 Windows 上，返回的接口名不能用作接口标识符，因为 Windows 使用数字形式的接口标识符，例如 `fe80::abc:def1:2345%5`。

要以跨平台兼容的方式获取接口标识符，请使用 `uv_if_indextoiid()`。

示例：

```c
char ifname[UV_IF_NAMESIZE];
size_t size = sizeof(ifname);
uv_if_indextoname(sin6->sin6_scope_id, ifname, &size);
```

*自 1.16.0 版本新增*

### int uv_if_indextoiid(unsigned int ifindex, char* buffer, size_t* size)

获取适用于 IPv6 带作用域地址的网络接口标识符。在 Windows 上，把数字形式的 `ifindex` 作为字符串返回。在所有其他平台上，会调用 `uv_if_indextoname()`。结果写入 `buffer`，`*size` 表示 `buffer` 的长度。如果 `buffer` 不足以容纳结果，返回 `UV_ENOBUFS`，且 `*size` 表示容纳结果所需的大小（包含 NUL 字节）。

更多细节参见 `uv_if_indextoname`。

*自 1.16.0 版本新增*

### int uv_exepath(char* buffer, size_t* size)

获取可执行文件路径。调用此函数之前你*必须*先调用 `uv_setup_args`。

在 setuid 可执行文件中要小心。在某些平台上，可执行文件路径是一个由用户控制的任意字符串。在另一些平台上，会查询可能处于用户控制之下的环境变量。

### int uv_cwd(char* buffer, size_t* size)

获取当前工作目录，并将其存入 `buffer`。如果当前工作目录太长而放不进 `buffer`，此函数返回 `UV_ENOBUFS`，并把 `size` 设置为所需的长度（包含 null 结尾符）。

*1.1.0 版本变更：在 Unix 上路径不再以斜杠结尾。*

*1.9.0 版本变更：返回 `UV_ENOBUFS` 时返回的长度包含结尾的 null 字节；成功时缓冲区以 null 结尾。*

### int uv_chdir(const char* dir)

改变当前工作目录。

### int uv_os_homedir(char* buffer, size_t* size)

获取当前用户的主目录。在 Windows 上，`uv_os_homedir()` 首先使用 `GetEnvironmentVariableW()` 检查 `USERPROFILE` 环境变量。如果 `USERPROFILE` 未设置，则调用 `GetUserProfileDirectoryW()`。在所有其他操作系统上，`uv_os_homedir()` 首先使用 `getenv(3)` 检查 `HOME` 环境变量。如果 `HOME` 未设置，则调用 `getpwuid_r(3)`。用户的主目录存入 `buffer`。调用 `uv_os_homedir()` 时，`size` 表示 `buffer` 的最大大小。成功时 `size` 被设为 `buffer` 的字符串长度。失败返回 `UV_ENOBUFS` 时，`size` 被设为 `buffer` 所需的长度（包含 null 字节）。

> **警告**：`uv_os_homedir()` 不是线程安全的。

*自 1.6.0 版本新增*

### int uv_os_tmpdir(char* buffer, size_t* size)

获取临时目录。在 Windows 上，`uv_os_tmpdir()` 使用 `GetTempPathW()`。在所有其他操作系统上，`uv_os_tmpdir()` 使用按 `TMPDIR`、`TMP`、`TEMP`、`TEMPDIR` 顺序找到的第一个环境变量。如果都找不到，则使用路径 `"/tmp"`，或在 Android 上使用 `"/data/local/tmp"`。临时目录存入 `buffer`。调用 `uv_os_tmpdir()` 时，`size` 表示 `buffer` 的最大大小。成功时 `size` 被设为 `buffer` 的字符串长度（不含结尾的 null）。失败返回 `UV_ENOBUFS` 时，`size` 被设为 `buffer` 所需的长度（包含 null 字节）。

> **警告**：`uv_os_tmpdir()` 不是线程安全的。

*自 1.9.0 版本新增*

### int uv_os_get_passwd(uv_passwd_t* pwd)

获取当前有效 uid（而非真实 uid）的密码文件条目的一个子集。填充的数据包括用户名、euid、gid、shell 和主目录。在非 Windows 系统上，所有数据来自 `getpwuid_r(3)`。在 Windows 上，uid 和 gid 被设为 -1 而没有意义，shell 为 `NULL`。成功调用此函数后，分配给 `pwd` 的内存需要用 `uv_os_free_passwd` 释放。

*自 1.9.0 版本新增*

### int uv_os_get_passwd2(uv_passwd_t* pwd, uv_uid_t uid)

获取所提供 uid 的密码文件条目的一个子集。填充的数据包括用户名、euid、gid、shell 和主目录。在非 Windows 系统上，所有数据来自 `getpwuid_r(3)`。在 Windows 上，uid 和 gid 被设为 -1 而没有意义，shell 为 `NULL`。成功调用此函数后，分配给 `pwd` 的内存需要用 `uv_os_free_passwd` 释放。

*自 1.45.0 版本新增*

### int uv_os_get_group(uv_group_t* group, uv_uid_t gid)

获取所提供 gid 的组文件条目的一个子集。填充的数据包括组名、gid 和成员。在非 Windows 系统上，所有数据来自 `getgrgid_r(3)`。在 Windows 上，uid 和 gid 被设为 -1 而没有意义。成功调用此函数后，分配给 `group` 的内存需要用 `uv_os_free_group` 释放。

*自 1.45.0 版本新增*

### void uv_os_free_group(uv_passwd_t* pwd)

释放之前由 `uv_os_get_group` 分配的内存。

*自 1.45.0 版本新增*

### void uv_os_free_passwd(uv_passwd_t* pwd)

释放之前由 `uv_os_get_passwd` 分配的 `pwd` 内存。

*自 1.9.0 版本新增*

### uint64_t uv_get_free_memory(void)

获取系统中可用的空闲内存量，以内核报告为准（字节）。未知时返回 0。

### uint64_t uv_get_total_memory(void)

获取系统中的物理内存总量（字节）。未知时返回 0。

### uint64_t uv_get_constrained_memory(void)

根据操作系统施加的限制，获取进程可用的内存总量（字节）。如果没有这样的限制，或限制未知，返回 `0`。如果存在限制机制但未设置限制，返回 `UINT64_MAX`。注意该值小于或大于 `uv_get_total_memory` 都并不罕见。

> **注意**：此函数目前在 Linux（如有 cgroups 则基于 cgroups）和 z/OS（基于 RLIMIT_MEMLIMIT）上才会返回非零值。

*自 1.29.0 版本新增*

### uint64_t uv_get_available_memory(void)

获取进程仍然可用的空闲内存量（字节）。它与 `uv_get_free_memory` 的区别在于它考虑了操作系统施加的任何限制。如果没有这样的限制，或限制未知，返回的量与 `uv_get_free_memory` 相同。

> **注意**：此函数目前在 Linux（如有 cgroups 则基于 cgroups）上才会返回与 `uv_get_free_memory` 不同的值。

*自 1.45.0 版本新增*

### uint64_t uv_hrtime(void)

返回当前的高精度时间戳。以纳秒表示。它相对于过去某个任意时刻。它与一天中的时间无关，因此不受时钟漂移影响。主要用途是测量区间之间的性能。

> **注意**：并非每个平台都能支持纳秒分辨率；但此值将始终以纳秒为单位。

### int uv_clock_gettime(uv_clock_id clock_id, uv_timespec64_t* ts)

从高精度的实时（real-time）或单调（monotonic）时钟源获取当前系统时间。

实时时钟从 UNIX 纪元（1970-01-01）起计数，且会受时间调整影响；它可能向后跳变。

单调时钟从过去某个任意时刻起计数，永远不会向后跳变。

*自 1.45.0 版本新增*

### void uv_print_all_handles(uv_loop_t* loop, FILE* stream)

将与给定 `loop` 关联的所有句柄打印到给定的 `stream`。

示例：

```c
uv_print_all_handles(uv_default_loop(), stderr);
/*
[--I] signal   0x1a25ea8
[-AI] async    0x1a25cf0
[R--] idle     0x1a7a8c8
*/
```

格式为 `[flags] handle-type handle-address`。对于 `flags`：

- 句柄被引用（referenced）时打印 `R`
- 句柄处于活动状态（active）时打印 `A`
- 句柄是内部句柄（internal）时打印 `I`

> **警告**：此函数用于临时调试，没有 API/ABI 稳定性保证。

*自 1.8.0 版本新增*

### void uv_print_active_handles(uv_loop_t* loop, FILE* stream)

与 `uv_print_all_handles` 相同，只是只打印活动句柄。

> **警告**：此函数用于临时调试，没有 API/ABI 稳定性保证。

*自 1.8.0 版本新增*

### int uv_os_environ(uv_env_item_t** envitems, int* count)

检索所有环境变量。此函数会分配内存，必须调用 `uv_os_free_environ` 释放。

> **警告**：此函数不是线程安全的。

*自 1.31.0 版本新增*

### void uv_os_free_environ(uv_env_item_t* envitems, int count);

释放 `uv_os_environ` 为环境变量分配的内存。

*自 1.31.0 版本新增*

### int uv_os_getenv(const char* name, char* buffer, size_t* size)

检索 `name` 指定的环境变量，把其值复制到 `buffer` 中，并把 `size` 设为该值的字符串长度。调用此函数时，`size` 必须被设置为 `buffer` 中可用的存储量（包含 null 结尾符）。如果环境变量的值超出 `buffer` 的可用存储量，返回 `UV_ENOBUFS`，并把 `size` 设为容纳该值所需的存储量。如果不存在匹配的环境变量，返回 `UV_ENOENT`。

> **警告**：此函数不是线程安全的。

*自 1.12.0 版本新增*

### int uv_os_setenv(const char* name, const char* value)

以 `value` 创建或更新 `name` 指定的环境变量。

> **警告**：此函数不是线程安全的。

*自 1.12.0 版本新增*

### int uv_os_unsetenv(const char* name)

删除 `name` 指定的环境变量。如果不存在这样的环境变量，此函数也会成功返回。

> **警告**：此函数不是线程安全的。

*自 1.12.0 版本新增*

### int uv_os_gethostname(char* buffer, size_t* size)

以 null 结尾字符串的形式在 `buffer` 中返回主机名，并把 `size` 设为主机名的字符串长度。调用此函数时，`size` 必须被设置为 `buffer` 中可用的存储量（包含 null 结尾符）。如果主机名超出 `buffer` 的可用存储量，返回 `UV_ENOBUFS`，并把 `size` 设为容纳该值所需的存储量。

*自 1.12.0 版本新增*

*1.26.0 版本变更：`UV_MAXHOSTNAMESIZE` 可用，它表示存储主机名和结尾 `nul` 字符所需的最大 `buffer` 大小。*

### int uv_os_getpriority(uv_pid_t pid, int* priority)

检索 `pid` 指定进程的调度优先级。返回的 `priority` 值介于 -20（高优先级）和 19（低优先级）之间。

> **注意**：在 Windows 上，返回的优先级将等于某个 `UV_PRIORITY` 常量。

*自 1.23.0 版本新增*

### int uv_os_setpriority(uv_pid_t pid, int priority)

设置 `pid` 指定进程的调度优先级。`priority` 值的范围介于 -20（高优先级）和 19（低优先级）之间。为方便起见还提供了常量 `UV_PRIORITY_LOW`、`UV_PRIORITY_BELOW_NORMAL`、`UV_PRIORITY_NORMAL`、`UV_PRIORITY_ABOVE_NORMAL`、`UV_PRIORITY_HIGH` 和 `UV_PRIORITY_HIGHEST`。

> **注意**：在 Windows 上，此函数使用 `SetPriorityClass()`。`priority` 参数被映射到一个 Windows 优先级类。检索进程优先级时，结果将等于某个 `UV_PRIORITY` 常量，而不一定是 `priority` 的精确值。

> **注意**：在 Windows 上，设置 `PRIORITY_HIGHEST` 只对提升过权限的用户有效，对其他用户会被静默降为 `PRIORITY_HIGH`。

> **注意**：在 IBM i PASE 上，最高进程优先级是 -10。常量 `UV_PRIORITY_HIGHEST` 为 -10，`UV_PRIORITY_HIGH` 为 -7，`UV_PRIORITY_ABOVE_NORMAL` 为 -4，`UV_PRIORITY_NORMAL` 为 0，`UV_PRIORITY_BELOW_NORMAL` 为 15，`UV_PRIORITY_LOW` 为 39。

> **注意**：在 IBM i PASE 上，除非你拥有 \*JOBCTL 特殊权限（special authority），否则不允许更改自己的优先级（即便调低也不行）。

*自 1.23.0 版本新增*

### int uv_os_uname(uv_utsname_t* buffer)

在 `buffer` 中检索系统信息。填充的数据包括操作系统名称、发行版、版本和机器类型。在非 Windows 系统上，`uv_os_uname()` 是 `uname(2)` 的一个薄封装。成功返回零，否则返回非零的错误值。

*自 1.25.0 版本新增*

### int uv_gettimeofday(uv_timeval64_t* tv)

`gettimeofday(2)` 的跨平台实现。`gettimeofday()` 的时区参数不受支持，因为它已被视为过时。

*自 1.28.0 版本新增*

### int uv_random(uv_loop_t* loop, uv_random_t* req, void* buf, size_t buflen, unsigned int flags, uv_random_cb cb)

用从系统 CSPRNG 获取的恰好 `buflen` 个密码学强随机字节填充 `buf`。`flags` 保留用于未来扩展，当前必须为 0。

不可能出现短读（short read）。当可用的随机字节少于 `buflen` 时，会返回非零的错误值或将其传给回调。

同步版本在熵不足时可能无限期阻塞。异步版本在系统熵低时可能永远不会完成。

熵的来源：

- Windows：`RtlGenRandom <https://docs.microsoft.com/en-us/windows/desktop/api/ntsecapi/nf-ntsecapi-rtlgenrandom>`_。
- Linux、Android：如有 `getrandom(2)` 则使用之，否则在从 `/dev/random` 读取一次后使用 `urandom(4)`，或使用 `KERN_RANDOM` `sysctl(2)`。
- FreeBSD：`getrandom(2) <https://www.freebsd.org/cgi/man.cgi?query=getrandom&sektion=2>`_，或在从 `/dev/random` 读取一次后使用 `/dev/urandom`。
- NetBSD：`KERN_ARND` `sysctl(7) <https://man.netbsd.org/sysctl.7>`_
- macOS、OpenBSD：如有 `getentropy(2) <https://man.openbsd.org/getentropy.2>`_ 则使用之，或在从 `/dev/random` 读取一次后使用 `/dev/urandom`。
- AIX：`/dev/random`。
- IBM i：`/dev/urandom`。
- 其他 UNIX：在从 `/dev/random` 读取一次后使用 `/dev/urandom`。

**返回值**：成功返回 0，失败返回小于 0 的错误码。出错后 `buf` 的内容是未定义的。

> **注意**：使用同步版本时，`loop` 和 `req` 两个参数都不使用，可以设为 `NULL`。

*自 1.33.0 版本新增*

### void uv_sleep(unsigned int msec)

使调用线程休眠 `msec` 毫秒。

*自 1.34.0 版本新增*

```c
#include <uv.h>
#include <stdio.h>
int main() {
    printf("Sleeping for 1 second...\n");
    uv_sleep(1000);
    printf("Awake!\n");
    return 0;
}
```

## 字符串操作函数

libuv 内部需要这些字符串工具来处理 Windows，并将其导出，以便在 libuv API 尚不完整时让客户端能够统一地处理这些数据。

### size_t uv_utf16_length_as_wtf8(const uint16_t* utf16, ssize_t utf16_len)

获取 `utf16` 值（UTF-16 或 UCS-2）转换为 WTF-8 之后的长度。如果 `utf16` 以 NUL 结尾，`utf16_len` 可以设为 -1；否则必须指定。

*自 1.47.0 版本新增*

### int uv_utf16_to_wtf8(const uint16_t* utf16, ssize_t utf16_len, char** wtf8_ptr, size_t* wtf8_len_ptr)

把 `utf16` 中的 UTF-16（或 UCS-2）数据转换为 `*wtf8_ptr` 中的 WTF-8 数据。`utf16_len` 计数（以字符为单位）给出 `utf16` 的长度。如果 `utf16` 以 NUL 结尾，`utf16_len` 可以设为 -1；否则必须指定。如果 `wtf8_ptr` 为 `NULL`，则不计算结果，但长度（等于 `uv_utf16_length_as_wtf8`）会存入 `wtf8_len_ptr`。如果 `*wtf8_ptr` 为 `NULL`，会为转换分配空间并经 `wtf8_ptr` 返回，长度经 `wtf8_len_ptr` 返回。否则，必须把 `*wtf8_ptr` 的长度经 `wtf8_len_ptr` 传入。`wtf8_ptr` 必须多留出一个空间用于结果之后的额外 NUL。如果结果被截断，将返回 `UV_ENOBUFS`，且 `wtf8_len_ptr` 为容纳完整结果所需的 `wtf8_ptr` 长度。

*自 1.47.0 版本新增*

### ssize_t uv_wtf8_length_as_utf16(const char* wtf8)

获取以 NUL 结尾的 WTF-8 `wtf8` 值转换为 UTF-16（或 UCS-2）之后的字符长度，包括 NUL 结尾符。

*自 1.47.0 版本新增*

### void uv_wtf8_to_utf16(const char* utf8, uint16_t* utf16, size_t utf16_len)

把 `wtf8` 中以 NUL 结尾的 WTF-8 数据转换为 `utf16` 中的 UTF-16（或 UCS-2）数据。`utf16_len` 计数（以字符为单位）必须包含 NUL 结尾符的空间。

*自 1.47.0 版本新增*

---

# 错误处理

在 libuv 中，错误是否为负数的常量。根据经验法则，只要出现状态参数，或某个 API 函数返回整数，负数就意味着出错。

当接受回调的函数返回错误时，回调将永远不会被调用。

> **注意**：实现细节：在 Unix 上错误码是对 `errno` 取负（即 `-errno`），而在 Windows 上它们由 libuv 定义为任意的负数。

## 错误常量

### UV_E2BIG

参数列表太长

### UV_EACCES

权限被拒绝

### UV_EADDRINUSE

地址已被占用

### UV_EADDRNOTAVAIL

地址不可用

### UV_EAFNOSUPPORT

不支持该地址族

### UV_EAGAIN

资源暂时不可用

### UV_EAI_ADDRFAMILY

不支持该地址族

### UV_EAI_AGAIN

临时失败

### UV_EAI_BADFLAGS

无效的 ai_flags 值

### UV_EAI_BADHINTS

hints 值无效

### UV_EAI_CANCELED

请求已取消

### UV_EAI_FAIL

永久性失败

### UV_EAI_FAMILY

不支持 ai_family

### UV_EAI_MEMORY

内存不足

### UV_EAI_NODATA

没有地址

### UV_EAI_NONAME

未知的节点或服务

### UV_EAI_OVERFLOW

参数缓冲区溢出

### UV_EAI_PROTOCOL

解析出的协议未知

### UV_EAI_SERVICE

该套接字类型没有可用的服务

### UV_EAI_SOCKTYPE

不支持该套接字类型

### UV_EALREADY

连接已在进行中

### UV_EBADF

无效的文件描述符

### UV_EBUSY

资源繁忙或被锁定

### UV_ECANCELED

操作已取消

### UV_ECHARSET

无效的 Unicode 字符

### UV_ECONNABORTED

软件导致的连接中止

### UV_ECONNREFUSED

连接被拒绝

### UV_ECONNRESET

连接被对端重置

### UV_EDESTADDRREQ

需要目标地址

### UV_EEXIST

文件已存在

### UV_EFAULT

系统调用参数中的地址错误

### UV_EFBIG

文件过大

### UV_EHOSTUNREACH

主机不可达

### UV_EINTR

系统调用被中断

### UV_EINVAL

无效的参数

### UV_EIO

I/O 错误

### UV_EISCONN

套接字已连接

### UV_EISDIR

对目录的非法操作

### UV_ELOOP

遇到过多符号链接

### UV_EMFILE

打开的文件过多

### UV_EMSGSIZE

消息过长

### UV_ENAMETOOLONG

名称过长

### UV_ENETDOWN

网络已断开

### UV_ENETUNREACH

网络不可达

### UV_ENFILE

文件表溢出

### UV_ENOBUFS

没有可用的缓冲区空间

### UV_ENODEV

没有这样的设备

### UV_ENOENT

没有这样的文件或目录

### UV_ENOMEM

内存不足

### UV_ENONET

机器不在网络上

### UV_ENOPROTOOPT

协议不可用

### UV_ENOSPC

设备上没有剩余空间

### UV_ENOSYS

函数未实现

### UV_ENOTCONN

套接字未连接

### UV_ENOTDIR

不是目录

### UV_ENOTEMPTY

目录非空

### UV_ENOTSOCK

对非套接字进行套接字操作

### UV_ENOTSUP

套接字不支持该操作

### UV_EOVERFLOW

值对于定义的数据类型而言过大

### UV_EPERM

操作不被允许

### UV_EPIPE

管道破裂

### UV_EPROTO

协议错误

### UV_EPROTONOSUPPORT

不支持该协议

### UV_EPROTOTYPE

套接字的协议类型错误

### UV_ERANGE

结果过大

### UV_EROFS

只读文件系统

### UV_ESHUTDOWN

传输端点关闭后无法发送

### UV_ESPIPE

无效的定位（seek）

### UV_ESRCH

没有这样的进程

### UV_ETIMEDOUT

连接超时

### UV_ETXTBSY

文本文件繁忙

### UV_EXDEV

不允许跨设备链接

### UV_UNKNOWN

未知错误

### UV_EOF

文件结束

### UV_ENXIO

没有这样的设备或地址

### UV_EMLINK

链接过多

### UV_ENOTTY

对该设备不适当的 ioctl 操作

### UV_EFTYPE

不适当的文件类型或格式

### UV_EILSEQ

非法的字节序列

### UV_ESOCKTNOSUPPORT

不支持该套接字类型

### UV_EUNATCH

协议驱动未附加

## API

### UV_ERRNO_MAP(iter_macro)

一个宏，它会对上述每个错误常量展开为一系列对 `iter_macro` 的调用。`iter_macro` 以两个参数被调用：不带 `UV_` 前缀的错误常量名，以及错误消息字符串字面量。

### const char* uv_strerror(int err)

返回给定错误码的错误消息。当用未知错误码调用它时会泄漏少量内存。

### char* uv_strerror_r(int err, char* buf, size_t buflen)

返回给定错误码的错误消息。以零结尾的消息被存入用户提供的、至多 `buflen` 字节的缓冲区 `buf` 中。

*自 1.22.0 版本新增*

### const char* uv_err_name(int err)

返回给定错误码的错误名称。当用未知错误码调用它时会泄漏少量内存。

### char* uv_err_name_r(int err, char* buf, size_t buflen)

返回给定错误码的错误名称。以零结尾的名称被存入用户提供的、至多 `buflen` 字节的缓冲区 `buf` 中。

*自 1.22.0 版本新增*

### int uv_translate_sys_error(int sys_errno)

返回与给定平台相关错误码等价的 libuv 错误码：在 Unix 上是 POSIX 错误码（存储在 `errno` 中的那些），在 Windows 上是 Win32 错误码（由 `GetLastError()` 或 `WSAGetLastError()` 返回的那些）。

如果 `sys_errno` 已经是 libuv 错误码，则原样返回。

*1.10.0 版本变更：函数被声明为公开。*

---

# 设计概述

libuv 是一个跨平台支持库，最初为 [Node.js](https://nodejs.org) 编写。它围绕事件驱动的异步 I/O 模型设计。

这个库提供的远不止是对不同 I/O 轮询机制的简单抽象：'句柄'（handles）和'流'（streams）为套接字和其他实体提供了高层抽象；此外还提供跨平台的文件 I/O 和线程功能等。

下面的图解说明了组成 libuv 的各个部分以及它们与各子系统的关系：

（图形版参见官方文档 static/architecture.png，文本版参见 static/architecture.txt。）

## 句柄与请求

libuv 为用户提供了两种与事件循环配合使用的抽象：句柄（handle）和请求（request）。

句柄表示长生命周期的对象，在其活动期间能够执行某些操作。一些例子：

- prepare（准备）句柄在活动期间每轮循环迭代都会触发一次回调。
- TCP 服务器句柄每有新连接时都会触发一次连接回调。

请求表示（通常）短生命周期的操作。这些操作可以在句柄之上执行：写请求用于在句柄上写数据；也可以独立存在：getaddrinfo 请求不需要句柄，它们直接在事件循环上运行。

处理句柄和请求的准则：

1. 如果 `uv_foo_init()` 成功初始化了句柄，就必须调用 `uv_close`。如果句柄的 init 函数报错，则无需做任何事情。

2. 只有句柄会被关闭，请求不会。例如，`uv_tcp_t` 是句柄，`uv_write_t` 是请求。

3. 句柄的内存只能在 `uv_close_cb` 内部或之后被回收或复用，不能更早。

4. 大多数句柄都有 init + start/stop 函数；有些句柄没有。例如 `uv_tcp_t` 与 `uv_process_t`；`uv_spawn` 把句柄初始化和进程启动合并成了一个。

5. 请求在完成时、或被 `uv_cancel` 取消时自动关闭。

6. 除 `uv_fs_t` 和 `uv_getaddrinfo_t` 请求外，无需额外的清理。对于 `uv_fs_t`，用完后调用 `uv_fs_req_cleanup`；对于 `uv_getaddrinfo_t`，则是 `uv_freeaddrinfo`。

7. 从那时起，请求的内存才能被回收或复用。

## I/O 循环

I/O（事件）循环是 libuv 的核心部分。它为所有 I/O 操作建立上下文，且被设计为绑定到单个线程。可以运行多个事件循环，只要每个运行在不同的线程中。libuv 的事件循环（以及任何其他涉及循环或句柄的 API）**不是线程安全的**，除非另有说明。

事件循环遵循相当常见的单线程异步 I/O 方式：所有（网络）I/O 都在非阻塞套接字上执行，并使用给定平台上可用的最佳机制进行轮询：Linux 上是 epoll，OS X 和其他 BSD 上是 kqueue，SunOS 上是 event ports，Windows 上是 IOCP。作为循环迭代的一部分，循环会阻塞等待已加入轮询器的套接字上的 I/O 活动，并触发回调来指示套接字状态（可读、可写、挂断），以便句柄能够读取、写入或执行所需的 I/O 操作。

为了更好地理解事件循环是如何运作的，下图说明了一轮循环迭代的全部阶段：

（图形版参见官方文档 static/loop_iteration.png，文本版参见 static/loop_iteration.txt。）

1. 循环的'now'概念被初始设定。

2. 如果循环以 ``UV_RUN_DEFAULT`` 模式运行，到期的定时器会被执行。所有计划时间早于循环 *now* 的活动定时器都会触发各自的回调。

3. 如果循环是*存活*的，则开始一轮迭代，否则循环立即退出。那么，何时认为循环是*存活*的呢？如果循环有活动的且被引用（ref'd）的句柄、活动请求或正在关闭的句柄，它就被认为是*存活*的。

4. 待处理的回调被调用。多数情况下，所有 I/O 回调都会在轮询 I/O 之后立即被调用。不过在某些情况下，此类回调的调用会被推迟到下一轮循环迭代。如果上一轮迭代推迟了任何 I/O 回调，它会在此刻运行。

5. idle（空闲）句柄回调被调用。尽管名字容易让人误会，只要处于活动状态，idle 句柄在每轮循环迭代中都会运行。

6. prepare（准备）句柄回调被调用。prepare 句柄的回调恰在循环因 I/O 而阻塞之前被触发。

7. 计算轮询超时。在因 I/O 阻塞之前，循环会计算它应该阻塞多久。计算超时的规则如下：

   - 如果循环以 ``UV_RUN_NOWAIT`` 标志运行，超时为 0。
   - 如果循环即将停止（已调用 `uv_stop`），超时为 0。
   - 如果没有活动的句柄或请求，超时为 0。
   - 如果有任何活动的 idle 句柄，超时为 0。
   - 如果有句柄正待关闭，超时为 0。
   - 如果以上情况都不匹配，则取最近的定时器的超时；如果没有活动定时器，则为无穷大。

8. 循环因 I/O 而阻塞。此时循环会按上一步计算出的时长阻塞等待 I/O。所有正在监视给定文件描述符读写操作的 I/O 相关句柄都会在此刻触发各自的回调。

9. check（检查）句柄回调被调用。check 句柄的回调恰在循环因 I/O 阻塞之后被触发。check 句柄本质上是 prepare 句柄的对应物。

10. 关闭回调被调用。如果句柄是通过调用 `uv_close` 关闭的，它的关闭回调会被调用。

11. 循环的'now'概念被更新。

12. 到期的定时器被执行。注意，'now' 在下一轮循环迭代之前不会再次更新。因此，如果某个定时器在其他定时器处理期间到期，它要等到下一轮事件循环迭代才会被执行。

13. 迭代结束。如果循环以 ``UV_RUN_NOWAIT`` 或 ``UV_RUN_ONCE`` 模式运行，迭代结束，`uv_run` 将返回。如果循环以 ``UV_RUN_DEFAULT`` 运行，只要它仍然*存活*就会从头继续，否则也将结束。

> **重要**：libuv 使用线程池来实现异步文件 I/O 操作，但网络 I/O **总是**在单线程（即每个循环各自的线程）中执行。

> **注意**：虽然轮询机制各不相同，libuv 使 Unix 系统和 Windows 之间的执行模型保持一致。

## 文件 I/O

与网络 I/O 不同，没有 libuv 可以依赖的平台特定的文件 I/O 原语，因此目前的方式是在线程池中运行阻塞式文件 I/O 操作。

关于跨平台文件 I/O 格局的详细说明，请参阅[这篇文章](https://blog.libtorrent.org/2012/10/asynchronous-disk-io/)。

libuv 目前使用一个全局线程池，所有循环都可以向它排队工作。目前在这个线程池上运行 3 类操作：

- 文件系统操作
- DNS 函数（getaddrinfo 和 getnameinfo）
- 通过 `uv_queue_work` 提交的用户指定代码

> **警告**：更多细节参见线程池一节，但请记住线程池的大小相当有限。

---

# 共享库处理

libuv 提供跨平台工具用于加载共享库并从中检索符号，使用以下 API。

## 数据类型

### uv_lib_t

共享库数据类型。

## 公开成员

无。

## API

### int uv_dlopen(const char* filename, uv_lib_t* lib)

打开一个共享库。文件名采用 utf-8。成功返回 0，出错返回 -1。调用 `uv_dlerror` 获取错误消息。

### void uv_dlclose(uv_lib_t* lib)

关闭共享库。

### int uv_dlsym(uv_lib_t* lib, const char* name, void** ptr)

从动态库中检索数据指针。符号映射到 NULL 是合法的。成功返回 0，未找到符号返回 -1。

### const char* uv_dlerror(const uv_lib_t* lib)

返回上一次 uv_dlopen() 或 uv_dlsym() 的错误消息。
