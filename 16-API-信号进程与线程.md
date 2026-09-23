# uv_signal_t — 信号句柄 · uv_process_t — 进程句柄 · 线程与同步工具

> 本章译自 libuv 官方文档 api/signal.rst、api/process.rst、api/threading.rst（libuv 1.52）

---

# uv_signal_t — 信号句柄

信号句柄以每个事件循环为单位实现 Unix 风格的信号处理。

## Windows 说明

部分信号的接收是模拟出来的：

- SIGINT 通常在用户按下 CTRL+C 时送达。不过与 Unix 一样，当终端处于原始（raw）模式时它不会被生成。

- SIGBREAK 在用户按下 CTRL + BREAK 时送达。

- SIGHUP 在用户关闭控制台窗口时生成。收到 SIGHUP 后，程序会有大约 10 秒的时间执行清理。之后 Windows 将无条件终止它。

- SIGWINCH 会在 libuv 检测到控制台大小被改变时触发。当 libuv 应用运行在控制台模拟器下，或当 32 位 libuv 应用运行在 64 位系统上时，SIGWINCH 会被模拟。在这些情况下，SIGWINCH 信号可能无法总是及时送达。对于一个可写的 `uv_tty_t` 句柄，libuv 只会在光标移动时才检测尺寸变化。当使用可读的 `uv_tty_t` 句柄时，只有当句柄处于原始模式且正在被读取时，才会检测到控制台缓冲区大小的改变。

- 其他信号的监视器可以成功创建，但这些信号永远不会被收到。这些信号是：`SIGILL`、`SIGABRT`、`SIGFPE`、`SIGSEGV`、`SIGTERM` 和 `SIGKILL`。

- 以编程方式调用 raise() 或 abort() 来触发的信号不会被 libuv 检测到；它们不会触发信号监视器。

*1.15.0 版本变更：改进了 Windows 上的 SIGWINCH 支持。*
*1.31.0 版本变更：64 位 Windows 上 32 位 libuv 的 SIGWINCH 支持回退到旧实现。*

## Unix 说明

- SIGKILL 和 SIGSTOP 无法被捕获。

- 通过 libuv 处理 SIGBUS、SIGFPE、SIGILL 或 SIGSEGV 会导致未定义行为。

- 由 `abort()`（例如通过 `assert()`）生成的 SIGABRT 不会被 libuv 捕获。

- 在 Linux 上，SIGRT0 和 SIGRT1（信号 32 和 33）被 NPTL pthreads 库用于管理线程。为这些信号安装监视器会导致不可预测的行为，强烈不建议这样做。libuv 的未来版本可能会直接拒绝它们。

## 数据类型

### uv_signal_t

信号句柄类型。

### void (*uv_signal_cb)(uv_signal_t* handle, int signum)

传给 `uv_signal_start` 的回调的类型定义。

## 公开成员

### int uv_signal_t.signum

此句柄正在监视的信号。只读。

> **参见**：`uv_handle_t` 的成员同样适用。

## API

### int uv_signal_init(uv_loop_t* loop, uv_signal_t* signal)

初始化句柄。

### int uv_signal_start(uv_signal_t* signal, uv_signal_cb cb, int signum)

以给定的回调启动句柄，监视给定的信号。

### int uv_signal_start_oneshot(uv_signal_t* signal, uv_signal_cb cb, int signum)

*自 1.12.0 版本新增*

功能与 `uv_signal_start` 相同，但信号在收到的瞬间监视器即被重置（仅触发一次）。

### int uv_signal_stop(uv_signal_t* signal)

停止句柄，回调将不再被调用。

> **参见**：`uv_handle_t` 的 API 函数同样适用。

---

# uv_process_t — 进程句柄

进程句柄用于派生新进程，并允许用户控制它、通过流与它建立通信信道。

## 数据类型

### uv_process_t

进程句柄类型。

### uv_process_options_t

派生进程的选项（传给 `uv_spawn`）。

```c
typedef struct uv_process_options_s {
    uv_exit_cb exit_cb;
    const char* file;
    char** args;
    char** env;
    const char* cwd;
    unsigned int flags;
    int stdio_count;
    uv_stdio_container_t* stdio;
    uv_uid_t uid;
    uv_gid_t gid;
} uv_process_options_t;
```

### void (*uv_exit_cb)(uv_process_t*, int64_t exit_status, int term_signal)

`uv_process_options_t` 中传入的回调的类型定义，它会给出退出状态以及导致进程终止的信号（如果有）。

### uv_process_flags

要设置在 `uv_process_options_t` 的 flags 字段上的标志。

```c
enum uv_process_flags {
    /*
    * 设置子进程的用户 id。
    */
    UV_PROCESS_SETUID = (1 << 0),
    /*
    * 设置子进程的组 id。
    */
    UV_PROCESS_SETGID = (1 << 1),
    /*
    * 在将参数列表转换为命令行字符串时，不给任何参数加引号，
    * 也不做任何其他转义。此选项仅在 Windows 系统上有意义。
    * 在 Unix 上会被静默忽略。
    */
    UV_PROCESS_WINDOWS_VERBATIM_ARGUMENTS = (1 << 2),
    /*
    * 以分离（detached）状态派生子进程——这会使它成为进程组组长，
    * 并实际允许子进程在父进程退出后继续运行。注意，除非父进程对
    * 子进程句柄调用 uv_unref()，否则子进程仍会保持父进程的
    * 事件循环存活。
    */
    UV_PROCESS_DETACHED = (1 << 3),
    /*
    * 隐藏通常会被创建的子进程窗口。此选项仅在 Windows 系统上有意义。
    * 在 Unix 上会被静默忽略。
    */
    UV_PROCESS_WINDOWS_HIDE = (1 << 4),
    /*
    * 隐藏通常会被创建的子进程控制台窗口。此选项仅在 Windows 系统上
    * 有意义。在 Unix 上会被静默忽略。
    */
    UV_PROCESS_WINDOWS_HIDE_CONSOLE = (1 << 5),
    /*
    * 隐藏通常会被创建的子进程 GUI 窗口。此选项仅在 Windows 系统上
    * 有意义。在 Unix 上会被静默忽略。
    */
    UV_PROCESS_WINDOWS_HIDE_GUI = (1 << 6),
    /*
     * 在 Windows 上，如果 uv_process_options_t 的 file 字段指定的
     * 待执行程序路径带有目录成分，则在尝试带 '.exe' 或 '.cmd' 等
     * 扩展名的变体之前，先搜索确切的文件名。
     */
    UV_PROCESS_WINDOWS_FILE_PATH_EXACT_NAME = (1 << 7)
};
```

### uv_stdio_container_t

传给子进程的每个 stdio 句柄或 fd 的容器。

```c
typedef struct uv_stdio_container_s {
    uv_stdio_flags flags;
    union {
        uv_stream_t* stream;
        int fd;
    } data;
} uv_stdio_container_t;
```

### uv_stdio_flags

指定如何将 stdio 传递给子进程的标志。

```c
typedef enum {
    /*
    * 以下四个选项互斥，用于定义对子进程中相应
    * 文件描述符执行的操作：
    */

    /*
    * 不提供文件描述符（如果是 fd 0、1 或 2，则重定向到 `/dev/null`）。
    */
    UV_IGNORE = 0x00,

    /*
    * 按照下面的标志打开一个新管道到 `data.stream`。`data.stream`
    * 字段必须指向一个已用 `uv_pipe_init(loop, data.stream, ipc);`
    * 初始化、但尚未打开或连接的 uv_pipe_t 对象。
    */
    UV_CREATE_PIPE = 0x01,

    /*
    * 子进程将获得父进程 `data.fd` 所指文件描述符的一个副本。
    */
    UV_INHERIT_FD = 0x02,

    /*
    * 子进程将获得父进程的 `data.stream` 句柄正在使用的文件描述符
    * 的一个副本。
    */
    UV_INHERIT_STREAM = 0x04,

    /*
    * 当指定 UV_CREATE_PIPE 时，UV_READABLE_PIPE 和 UV_WRITABLE_PIPE
    * 从子进程的视角决定数据流的方向。两个标志可以同时指定，
    * 以创建双工数据流。
    */
    UV_READABLE_PIPE = 0x10,
    UV_WRITABLE_PIPE = 0x20,

    /*
    * 当指定 UV_CREATE_PIPE 时，再指定 UV_NONBLOCK_PIPE 会在子进程中
    * 以非阻塞模式打开该句柄。如果子进程的设计没有考虑遇到这种模式，
    * 可能会导致数据丢失，但也可以显著提高效率。
    */
    UV_NONBLOCK_PIPE = 0x40
} uv_stdio_flags;
```

## 公开成员

### int uv_process_t.pid

被派生进程的 PID。它在调用 `uv_spawn` 之后被设置，并且在进程退出后仍保留该值。该值仅在进程存活期间唯一；进程退出后，另一个进程可能被重新分配相同的 PID。

> **注意**：`uv_handle_t` 的成员同样适用。

### uv_exit_cb uv_process_options_t.exit_cb

进程退出后调用的回调。

### const char* uv_process_options_t.file

指向要执行的程序的路径。

### char** uv_process_options_t.args

命令行参数。args[0] 应为程序的路径。在 Windows 上这会使用 `CreateProcess`，它把参数拼接成一个字符串，这可能引发一些奇怪的错误。参见 `uv_process_flags` 中的 `UV_PROCESS_WINDOWS_VERBATIM_ARGUMENTS` 标志。

### char** uv_process_options_t.env

新进程的环境变量。如果为 NULL，则使用父进程的环境变量。

### const char* uv_process_options_t.cwd

子进程的当前工作目录。

### unsigned int uv_process_options_t.flags

控制 `uv_spawn` 行为的各种标志。参见 `uv_process_flags`。

### int uv_process_options_t.stdio_count
### uv_stdio_container_t* uv_process_options_t.stdio

`stdio` 字段指向一个 `uv_stdio_container_t` 结构体数组，描述将提供给子进程使用的文件描述符。约定是 stdio[0] 指向 stdin，fd 1 用于 stdout，fd 2 为 stderr。

> **注意**：在 Windows 上，只有当子进程使用 MSVCRT 运行时，大于 2 的文件描述符才对子进程可用。

### uv_uid_t uv_process_options_t.uid
### uv_gid_t uv_process_options_t.gid

libuv 可以改变子进程的用户/组 id。只有当 flags 字段中设置了相应的位时才会发生。

> **注意**：Windows 不支持此功能，`uv_spawn` 会失败并把错误设置为 `UV_ENOTSUP`。

### uv_stdio_flags uv_stdio_container_t.flags

指定 stdio 容器如何传递给子进程的标志。

### union @0 uv_stdio_container_t.data

包含要传给子进程的 `stream` 或 `fd` 的联合体。

## API

### void uv_disable_stdio_inheritance(void)

禁用本进程从其父进程继承的文件描述符/句柄的继承性。其效果是由本进程派生的子进程不会意外继承这些句柄。

建议尽早在程序中调用此函数，最好在继承来的文件描述符可能被关闭或复制之前。

> **注意**：此函数尽力而为：不保证 libuv 能发现所有被继承的文件描述符。一般而言它在 Windows 上做得比在 Unix 上好。

### int uv_spawn(uv_loop_t* loop, uv_process_t* handle, const uv_process_options_t* options)

初始化进程句柄并启动进程。如果进程成功派生，此函数返回 0。否则返回对应失败原因的负数错误码。注意无论成功还是失败——你最终都必须调用 `uv_close` 关闭句柄，然后才能释放句柄的内存，这与 libuv 中其他的 init 函数不同。

派生失败的可能原因包括（但不限于）：要执行的文件不存在、没有权限使用指定的 setuid 或 setgid，或者没有足够的内存分配给新进程。

> **警告**：在 Unix 上，如果调用 `uv_close` 时进程尚未退出，你会制造一个 libuv 无法回收的僵尸进程。之后你需要负责调用 `waitpid`。这在 Windows 上不适用。

*1.24.0 版本变更：新增 `UV_PROCESS_WINDOWS_HIDE_CONSOLE` 和 `UV_PROCESS_WINDOWS_HIDE_GUI` 标志。*

*1.48.0 版本变更：新增 `UV_PROCESS_WINDOWS_FILE_PATH_EXACT_NAME` 标志。*

### int uv_process_kill(uv_process_t* handle, int signum)

向给定的进程句柄发送指定的信号。关于信号支持情况（特别是在 Windows 上）请查阅信号一章的文档。

如果指定的进程已经死亡，这不会杀掉恰好复用了同一 pid 的另一个进程。相比之下，如果你使用缓存的 `uv_process_get_pid` 值，`uv_kill` 可能会杀掉任意的其他进程。

### int uv_kill(int pid, int signum)

向给定的 PID 发送指定的信号。关于信号支持情况（特别是在 Windows 上）请查阅信号一章的文档。

### uv_pid_t uv_process_get_pid(const uv_process_t* handle)

返回 `handle->pid`。

*自 1.19.0 版本新增*

> **参见**：`uv_handle_t` 的 API 函数同样适用。

---

# 线程与同步工具

libuv 为多种线程和同步原语提供了跨平台实现。该 API 在很大程度上遵循 pthreads API。

## 数据类型

### uv_thread_t

线程数据类型。

### void (*uv_thread_cb)(void* arg)

用于启动线程执行的回调。`arg` 与传给 `uv_thread_create` 的值相同。

### uv_key_t

线程局部存储键（thread-local key）数据类型。

### uv_once_t

一次性初始化器数据类型。

### uv_mutex_t

互斥锁数据类型。

### uv_rwlock_t

读写锁数据类型。

### uv_sem_t

信号量数据类型。

### uv_cond_t

条件变量数据类型。

### uv_barrier_t

屏障（barrier）数据类型。

## API

### 线程

#### uv_thread_options_t

派生新线程的选项（传给 `uv_thread_create_ex`）。

```c
typedef struct uv_thread_options_s {
  enum {
    UV_THREAD_NO_FLAGS = 0x00,
    UV_THREAD_HAS_STACK_SIZE = 0x01
  } flags;
  size_t stack_size;
} uv_thread_options_t;
```

此结构体随时可能添加更多字段，因此不应依赖其确切的布局和大小。

*自 1.26.0 版本新增*

#### int uv_thread_create(uv_thread_t* tid, uv_thread_cb entry, void* arg)

*1.4.1 版本变更：失败时返回 UV_E* 错误码。*

#### int uv_thread_detach(uv_thread_t* tid)

分离一个线程。被分离的线程在终止时会自动释放自己的资源，应用无需再调用 `uv_thread_join`。

*自 1.50.0 版本新增*

#### int uv_thread_create_ex(uv_thread_t* tid, const uv_thread_options_t* params, uv_thread_cb entry, void* arg)

与 `uv_thread_create` 类似，但额外指定创建新线程的选项。

如果设置了 `UV_THREAD_HAS_STACK_SIZE`，`stack_size` 为新线程指定栈大小。`0` 表示应使用默认值，即行为如同未设置该标志。其他值会被向上取整到最近的页边界。

*自 1.26.0 版本新增*

#### int uv_thread_setaffinity(uv_thread_t* tid, char* cpumask, char* oldmask, size_t mask_size)

将指定线程的亲和性设置为 cpumask，它以字节为单位指定。可以选择在 oldmask 中返回之前的亲和性设置。在 Unix 上，使用 `pthread_getaffinity_np(3)` 获取亲和性设置，并把 cpu_set_t 映射到 oldmask 中的字节。然后把 cpumask 中的字节映射到 cpu_set_t 并使用 `pthread_setaffinity_np(3)`。在 Windows 上，把 cpumask 中的字节映射到位掩码并使用 SetThreadAffinityMask()，它会返回之前的亲和性设置。

`mask_size` 指定 cpumask/oldmask 中条目（字节）的数量，必须大于或等于 `uv_cpumask_size`。

> **注意**：在 Windows 上线程亲和性设置不是原子操作。macOS 上不支持。

*自 1.45.0 版本新增*

#### int uv_thread_getaffinity(uv_thread_t* tid, char* cpumask, size_t mask_size)

获取指定线程的亲和性设置。在 Unix 上，这会把 `pthread_getaffinity_np(3)` 返回的 cpu_set_t 映射到 cpumask 中的字节。

`mask_size` 指定 cpumask 中条目（字节）的数量，必须大于或等于 `uv_cpumask_size`。

> **注意**：在 Windows 上线程亲和性获取不是原子操作。macOS 上不支持。

*自 1.45.0 版本新增*

#### int uv_thread_getcpu(void)

获取调用线程当前运行所在的 CPU 编号。

> **注意**：目前仅在 Windows、Linux 和 FreeBSD 上实现。

*自 1.45.0 版本新增*

#### uv_thread_t uv_thread_self(void)
#### int uv_thread_join(uv_thread_t *tid)
#### int uv_thread_equal(const uv_thread_t* t1, const uv_thread_t* t2)

#### int uv_thread_setname(const char* name)

设置当前线程的名称。不同平台对线程名称的最大字符数定义了不同的限制：Linux、IBM i（16）、macOS（64）、Windows（32767）和 NetBSD（32）等。如果 `name` 超出平台的限制，`uv_thread_setname()` 会将其截断。

Windows Server 2016 上不支持，返回 `UV_ENOSYS`。

*自 1.50.0 版本新增*

#### int uv_thread_getname(uv_thread_t* tid, char* name, size_t* size)

获取 `tid` 指定线程的名称。线程名称（连同结尾的 NUL）会被复制到 `name` 指向的缓冲区。`size` 参数指定 `name` 所指缓冲区的大小。缓冲区应足够容纳线程名称加上结尾的 NUL，否则将被截断以适应（并带结尾 NUL）。

在线程已终止之后调用此函数是未定义行为。

Windows Server 2016 上不支持，返回 `UV_ENOSYS`。

*自 1.50.0 版本新增*

#### int uv_thread_setpriority(uv_thread_t tid, int priority)

函数成功时返回值为 0。函数失败时返回值小于 0。设置 tid 指定线程的调度优先级。在某些平台上设置特定优先级需要提升的权限。优先级可以设置为以下常量：UV_THREAD_PRIORITY_HIGHEST、UV_THREAD_PRIORITY_ABOVE_NORMAL、UV_THREAD_PRIORITY_NORMAL、UV_THREAD_PRIORITY_BELOW_NORMAL、UV_THREAD_PRIORITY_LOWEST。

在线程已终止之后调用此函数是未定义行为。

*自 1.48.0 版本新增*

#### int uv_thread_getpriority(uv_thread_t tid, int* priority)

函数成功时返回值为 0。函数失败时返回值小于 0。获取 tid 指定线程的调度优先级。输出参数 priority 中的值与平台相关。在 Linux 上，当调度策略为 SCHED_OTHER（默认）时，priority 为 0。

在线程已终止之后调用此函数是未定义行为。

*自 1.48.0 版本新增*

### 线程局部存储

> **注意**：线程局部存储的总大小可能是有限的。也就是说，可能无法创建很多 TLS 键。

#### int uv_key_create(uv_key_t* key)
#### void uv_key_delete(uv_key_t* key)
#### void* uv_key_get(uv_key_t* key)
#### void uv_key_set(uv_key_t* key, void* value)

### 一次性初始化

只运行一次某函数。以同一个 guard 并发调用 `uv_once` 时，除一个调用者外（未指定是哪一个），所有调用者都会被阻塞。guard 应使用 UV_ONCE_INIT 宏进行静态初始化。

#### void uv_once(uv_once_t* guard, void (*callback)(void))

### 互斥锁

这些函数成功时返回 0，失败时返回小于 0 的错误码（当然，返回类型为 void 的除外）。

#### int uv_mutex_init(uv_mutex_t* handle)
#### int uv_mutex_init_recursive(uv_mutex_t* handle)
#### void uv_mutex_destroy(uv_mutex_t* handle)
#### void uv_mutex_lock(uv_mutex_t* handle)
#### int uv_mutex_trylock(uv_mutex_t* handle)
#### void uv_mutex_unlock(uv_mutex_t* handle)

### 读写锁

这些函数成功时返回 0，失败时返回小于 0 的错误码（当然，返回类型为 void 的除外）。

#### int uv_rwlock_init(uv_rwlock_t* rwlock)
#### void uv_rwlock_destroy(uv_rwlock_t* rwlock)
#### void uv_rwlock_rdlock(uv_rwlock_t* rwlock)
#### int uv_rwlock_tryrdlock(uv_rwlock_t* rwlock)
#### void uv_rwlock_rdunlock(uv_rwlock_t* rwlock)
#### void uv_rwlock_wrlock(uv_rwlock_t* rwlock)
#### int uv_rwlock_trywrlock(uv_rwlock_t* rwlock)
#### void uv_rwlock_wrunlock(uv_rwlock_t* rwlock)

### 信号量

这些函数成功时返回 0，失败时返回小于 0 的错误码（当然，返回类型为 void 的除外）。

#### int uv_sem_init(uv_sem_t* sem, unsigned int value)
#### void uv_sem_destroy(uv_sem_t* sem)
#### void uv_sem_post(uv_sem_t* sem)
#### void uv_sem_wait(uv_sem_t* sem)
#### int uv_sem_trywait(uv_sem_t* sem)

### 条件变量

这些函数成功时返回 0，失败时返回小于 0 的错误码（当然，返回类型为 void 的除外）。

> **注意**：
> 1. 调用者应准备好应对 `uv_cond_wait` 和 `uv_cond_timedwait` 上的虚假唤醒（spurious wakeups）。
> 2. `uv_cond_timedwait` 的超时参数是相对于函数被调用的时刻的。
> 3. 在 z/OS 上，`uv_cond_timedwait` 的超时参数会被转换为等待过期的绝对系统时间。如果当前系统时钟时间在条件被通知之前就超过了计算出的绝对时间，则产生 ETIMEDOUT 错误。等待开始后，等待时间不受系统时钟变更的影响。

#### int uv_cond_init(uv_cond_t* cond)
#### void uv_cond_destroy(uv_cond_t* cond)
#### void uv_cond_signal(uv_cond_t* cond)
#### void uv_cond_broadcast(uv_cond_t* cond)
#### void uv_cond_wait(uv_cond_t* cond, uv_mutex_t* mutex)
#### int uv_cond_timedwait(uv_cond_t* cond, uv_mutex_t* mutex, uint64_t timeout)

### 屏障

这些函数成功时返回 0，失败时返回小于 0 的错误码（当然，返回类型为 void 的除外）。

> **注意**：`uv_barrier_wait` 会向一个任意选定的"序列化者"（serializer）线程返回大于 0 的值，以便于进行清理，即：

```c
if (uv_barrier_wait(&barrier) > 0)
    uv_barrier_destroy(&barrier);
```

#### int uv_barrier_init(uv_barrier_t* barrier, unsigned int count)
#### void uv_barrier_destroy(uv_barrier_t* barrier)
#### int uv_barrier_wait(uv_barrier_t* barrier)
