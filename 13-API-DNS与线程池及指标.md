# DNS、线程池、指标与版本（DNS utility functions、Thread pool、Metrics、Version）

> 本章译自 libuv 官方文档 api/dns.rst、api/threadpool.rst、api/metrics.rst、api/version.rst（libuv 1.52）

## DNS 工具函数（DNS utility functions）

libuv 提供了 `getaddrinfo` 和 `getnameinfo` 的异步版本。

### 数据类型

#### `uv_getaddrinfo_t`

`getaddrinfo` 请求类型。

#### `void (*uv_getaddrinfo_cb)(uv_getaddrinfo_t* req, int status, struct addrinfo* res)`

域名解析请求完成后携带结果调用的回调。如果请求被取消，`status` 的值将为 `UV_ECANCELED`。

#### `uv_getnameinfo_t`

`getnameinfo` 请求类型。

#### `void (*uv_getnameinfo_cb)(uv_getnameinfo_t* req, int status, const char* hostname, const char* service)`

名称解析请求完成后携带结果调用的回调。如果请求被取消，`status` 的值将为 `UV_ECANCELED`。

### 公开成员

#### `uv_loop_t* uv_getaddrinfo_t.loop`

启动该域名解析请求并在其中报告完成事件的循环。只读。

#### `struct addrinfo* uv_getaddrinfo_t.addrinfo`

指向包含结果的 `struct addrinfo` 的指针。必须由用户使用 `uv_freeaddrinfo` 释放。

> 1.3.0 变更：该字段被声明为公开。

#### `uv_loop_t* uv_getnameinfo_t.loop`

启动该名称解析请求并在其中报告完成事件的循环。只读。

#### `char[NI_MAXHOST] uv_getnameinfo_t.host`

包含结果主机名的字符数组。以 null 结尾。

> 1.3.0 变更：该字段被声明为公开。

#### `char[NI_MAXSERV] uv_getnameinfo_t.service`

包含结果服务名的字符数组。以 null 结尾。

> 1.3.0 变更：该字段被声明为公开。

> **参见**：`uv_req_t` 的成员同样适用。

### API

#### `int uv_getaddrinfo(uv_loop_t* loop, uv_getaddrinfo_t* req, uv_getaddrinfo_cb getaddrinfo_cb, const char* node, const char* service, const struct addrinfo* hints)`

```c
int uv_getaddrinfo(uv_loop_t* loop, uv_getaddrinfo_t* req, uv_getaddrinfo_cb getaddrinfo_cb, const char* node, const char* service, const struct addrinfo* hints)
```

异步的 `getaddrinfo(3)`。

`node` 和 `service` 可以有一个为 NULL，但不能都为 NULL。

`hints` 是指向 `struct addrinfo` 的指针，包含额外的地址类型约束，也可以为 NULL。更多细节请查阅 `man -s 3 getaddrinfo`。

成功返回 0，失败返回错误码 < 0。如果成功，回调将在未来的某个时刻被调用，并携带解析结果，结果为以下二者之一：

- status == 0，`res` 参数指向一个有效的 `struct addrinfo`；或
- status < 0，`res` 参数为 NULL。参见 `UV_EAI_*` 常量。

调用 `uv_freeaddrinfo` 释放 addrinfo 结构。

> 1.3.0 变更：回调参数现在允许为 NULL，此时请求将**同步**运行。

#### `void uv_freeaddrinfo(struct addrinfo* ai)`

```c
void uv_freeaddrinfo(struct addrinfo* ai)
```

释放 `struct addrinfo`。允许传入 NULL，此时为无操作。

#### `int uv_getnameinfo(uv_loop_t* loop, uv_getnameinfo_t* req, uv_getnameinfo_cb getnameinfo_cb, const struct sockaddr* addr, int flags)`

```c
int uv_getnameinfo(uv_loop_t* loop, uv_getnameinfo_t* req, uv_getnameinfo_cb getnameinfo_cb, const struct sockaddr* addr, int flags)
```

异步的 `getnameinfo(3)`。

成功返回 0，失败返回错误码 < 0。如果成功，回调将在未来的某个时刻被调用，并携带解析结果。更多细节请查阅 `man -s 3 getnameinfo`。

> 1.3.0 变更：回调参数现在允许为 NULL，此时请求将**同步**运行。

> **参见**：`uv_req_t` 的 API 函数同样适用。

---

## 线程池工作调度（Thread pool work scheduling）

libuv 提供了一个线程池，可用于运行用户代码并在循环线程中得到通知。该线程池在内部用于运行所有文件系统操作，以及域名解析（getaddrinfo）和名称解析（getnameinfo）请求。

其默认大小为 4，但可以在启动时通过将 `UV_THREADPOOL_SIZE` 环境变量设置为任意值来更改（绝对上限为 1024）。

> 1.30.0 变更：允许的 `UV_THREADPOOL_SIZE` 最大值从 128 提高到 1024。

> 1.45.0 变更：线程现在拥有 8 MB 的栈，而不是（有时过低的）平台默认值。

> 1.50.0 变更：线程现在拥有默认名称 libuv-worker。

线程池是全局的，在所有事件循环之间共享。当某个特定函数使用线程池时（例如使用 `uv_queue_work`），libuv 会预分配并初始化 `UV_THREADPOOL_SIZE` 允许的最大线程数。更多的线程通常意味着更高的吞吐量，但也会带来更高的内存占用。不过在大多数平台上，线程栈是惰性增长的。

> **注意**：请注意，尽管使用的是跨所有事件循环共享的全局线程池，这些函数本身并不是线程安全的。

### 数据类型

#### `uv_work_t`

工作请求类型。

#### `void (*uv_work_cb)(uv_work_t* req)`

传给 `uv_queue_work` 的回调，将在线程池上运行。

#### `void (*uv_after_work_cb)(uv_work_t* req, int status)`

传给 `uv_queue_work` 的回调，在线程池上的工作完成后于循环线程中被调用。如果工作通过 `uv_cancel` 被取消，`status` 将为 `UV_ECANCELED`。

### 公开成员

#### `uv_loop_t* uv_work_t.loop`

启动该请求并在其中报告完成事件的循环。只读。

> **参见**：`uv_req_t` 的成员同样适用。

### API

#### `int uv_queue_work(uv_loop_t* loop, uv_work_t* req, uv_work_cb work_cb, uv_after_work_cb after_work_cb)`

```c
int uv_queue_work(uv_loop_t* loop, uv_work_t* req, uv_work_cb work_cb, uv_after_work_cb after_work_cb)
```

初始化一个工作请求，它将在线程池中的某个线程上运行给定的 `work_cb`。`work_cb` 完成后，`after_work_cb` 将在循环线程中被调用。

该请求可以通过 `uv_cancel` 取消。

> **参见**：`uv_req_t` 的 API 函数同样适用。

---

## 指标操作（Metrics operations）

libuv 提供了一套指标 API，用于跟踪事件循环的各种内部操作。

### 数据类型

#### `uv_metrics_t`

包含事件循环指标的结构体。建议在 `uv_prepare_cb` 中获取这些指标，以确保指标计数器之间不存在不一致。

```c
typedef struct {
    uint64_t loop_count;
    uint64_t events;
    uint64_t events_waiting;
    /* private */
    uint64_t* reserved[13];
} uv_metrics_t;
```

### 公开成员

#### `uint64_t uv_metrics_t.loop_count`

事件循环迭代的次数。

#### `uint64_t uv_metrics_t.events`

事件处理器已处理的事件数量。

#### `uint64_t uv_metrics_t.events_waiting`

调用事件提供者（event provider）时正等待处理的事件数量。

### API

#### `uint64_t uv_metrics_idle_time(uv_loop_t* loop)`

```c
uint64_t uv_metrics_idle_time(uv_loop_t* loop)
```

获取事件循环在内核事件提供者（例如 `epoll_wait`）中处于空闲状态的时长。该调用是线程安全的。

返回值是自 `uv_loop_t` 被配置为收集空闲时间以来，在内核事件提供者中累计的空闲时间。

> **注意**：在以 `UV_METRICS_IDLE_TIME` 调用 `uv_loop_configure` 之前，事件循环不会开始累计事件提供者的空闲时间。

> 1.39.0 新增。

#### `int uv_metrics_info(uv_loop_t* loop, uv_metrics_t* metrics)`

```c
int uv_metrics_info(uv_loop_t* loop, uv_metrics_t* metrics)
```

将当前的事件循环指标集合复制到 `metrics` 指针。

> 1.45.0 新增。

---

## 版本检查宏与函数（Version-checking macros and functions）

从 1.0.0 版本开始，libuv 遵循[语义化版本（semantic versioning）](https://semver.org)方案。这意味着在一个主版本的生命周期内可以引入新 API。本节列出了所有可用于条件编译或条件执行代码的宏和函数，以便兼容多个 libuv 版本。

### 宏

#### `UV_VERSION_MAJOR`

libuv 版本的主版本号。

#### `UV_VERSION_MINOR`

libuv 版本的次版本号。

#### `UV_VERSION_PATCH`

libuv 版本的修订号。

#### `UV_VERSION_IS_RELEASE`

设为 1 表示 libuv 的正式发布版本，0 表示开发快照。

#### `UV_VERSION_SUFFIX`

libuv 版本后缀。某些开发版本（如发布候选版本）可能带有诸如 "rc" 的后缀。

#### `UV_VERSION_HEX`

返回打包为单个整数的 libuv 版本。每个组件占 8 位，修订号存储在最低的 8 位中。例如 libuv 1.2.3 对应 0x010203。

> 1.7.0 新增。

### 函数

#### `unsigned int uv_version(void)`

```c
unsigned int uv_version(void)
```

返回 `UV_VERSION_HEX`。

#### `const char* uv_version_string(void)`

```c
const char* uv_version_string(void)
```

以字符串形式返回 libuv 版本号。对于非正式发布版本，版本后缀会被包含在内。
