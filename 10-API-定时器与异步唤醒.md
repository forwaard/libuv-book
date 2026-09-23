> 本章译自 libuv 官方文档 api/timer.rst、api/check.rst、api/prepare.rst、api/idle.rst 与 api/async.rst（libuv 1.52）

# 第一部分 `uv_timer_t` — 定时器句柄

定时器句柄用于安排回调在将来被调用。

定时器既可以是一次性的，也可以是重复的。重复定时器不会根据执行开销进行调整，而是相对于事件循环所理解的"现在"重新装载。

libuv 会在执行定时器回调之前、以及从等待 I/O 中唤醒之后，立即更新它对"现在"的理解。另见 `uv_update_time()`。

示例：一个间隔为 50 ms 的重复定时器，若其回调耗时 17 ms 完成，则会在 33 ms 后再次运行。如果其他任务耗时超过 33 ms，定时器回调会尽可能快地运行。

## 数据类型

### `uv_timer_t`

定时器句柄类型。

### `void (*uv_timer_cb)(uv_timer_t* handle)`

传递给 `uv_timer_start()` 的回调的类型定义。

### 公共成员

无。

> **参见**：`uv_handle_t` 的成员同样适用。

## API

### `int uv_timer_init(uv_loop_t* loop, uv_timer_t* handle)`

初始化句柄。

### `int uv_timer_start(uv_timer_t* handle, uv_timer_cb cb, uint64_t timeout, uint64_t repeat)`

启动定时器。`timeout` 和 `repeat` 的单位是毫秒。

如果 `timeout` 为零，回调将在下一次事件循环迭代时触发。如果 `repeat` 非零，回调先在 `timeout` 毫秒后触发，之后每隔 `repeat` 毫秒重复触发。

> **注意**：不会更新事件循环所理解的"现在"。更多信息参见 `uv_update_time()`。

> **注意**：如果定时器已经处于活动状态，则只是被更新。

### `int uv_timer_stop(uv_timer_t* handle)`

停止定时器，回调将不再被调用。

### `int uv_timer_again(uv_timer_t* handle)`

停止定时器，如果它是重复定时器，则以 repeat 值作为超时重新启动它。如果定时器从未被启动过，则返回 UV_EINVAL。

### `void uv_timer_set_repeat(uv_timer_t* handle, uint64_t repeat)`

以毫秒为单位设置重复间隔值。定时器将按给定间隔被调度运行，而不受回调执行时长的影响；在发生时间片超限的情况下，将遵循正常的定时器语义。

> **注意**：如果在定时器回调中设置 repeat 值，它不会立即生效。如果定时器此前是非重复的，它已经停止；如果它是重复的，下一次超时将使用旧的 repeat 值来调度。

### `uint64_t uv_timer_get_repeat(const uv_timer_t* handle)`

获取定时器的 repeat 值。

### `uint64_t uv_timer_get_due_in(const uv_timer_t* handle)`

获取定时器的到期剩余时间；若已到期则返回 0。该时间相对于 `uv_now()`。

> **注意**：自 1.40.0 版本起新增。

> **参见**：`uv_handle_t` 的 API 函数同样适用。

---

# 第二部分 `uv_check_t` — Check 句柄

Check 句柄会在每次循环迭代中运行给定的回调一次，时机紧随 I/O 轮询之后。

## 数据类型

### `uv_check_t`

Check 句柄类型。

### `void (*uv_check_cb)(uv_check_t* handle)`

传递给 `uv_check_start()` 的回调的类型定义。

### 公共成员

无。

> **参见**：`uv_handle_t` 的成员同样适用。

## API

### `int uv_check_init(uv_loop_t* loop, uv_check_t* check)`

初始化句柄。此函数总是成功。

返回值：0。

### `int uv_check_start(uv_check_t* check, uv_check_cb cb)`

以给定的回调启动句柄。此函数总是成功，除非 `cb` 为 `NULL`。

返回值：成功时为 0，当 `cb == NULL` 时为 `UV_EINVAL`。

### `int uv_check_stop(uv_check_t* check)`

停止句柄，回调将不再被调用。此函数总是成功。

返回值：0。

> **参见**：`uv_handle_t` 的 API 函数同样适用。

---

# 第三部分 `uv_prepare_t` — Prepare 句柄

Prepare 句柄会在每次循环迭代中运行给定的回调一次，时机紧接在 I/O 轮询之前。

## 数据类型

### `uv_prepare_t`

Prepare 句柄类型。

### `void (*uv_prepare_cb)(uv_prepare_t* handle)`

传递给 `uv_prepare_start()` 的回调的类型定义。

### 公共成员

无。

> **参见**：`uv_handle_t` 的成员同样适用。

## API

### `int uv_prepare_init(uv_loop_t* loop, uv_prepare_t* prepare)`

初始化句柄。此函数总是成功。

返回值：0。

### `int uv_prepare_start(uv_prepare_t* prepare, uv_prepare_cb cb)`

以给定的回调启动句柄。此函数总是成功，除非 `cb` 为 `NULL`。

返回值：成功时为 0，当 `cb == NULL` 时为 `UV_EINVAL`。

### `int uv_prepare_stop(uv_prepare_t* prepare)`

停止句柄，回调将不再被调用。此函数总是成功。

返回值：0。

> **参见**：`uv_handle_t` 的 API 函数同样适用。

---

# 第四部分 `uv_idle_t` — Idle 句柄

Idle 句柄会在每次循环迭代中运行给定的回调一次，时机在 `uv_prepare_t` 句柄之前。

> **注意**：与 Prepare 句柄的显著区别在于：当存在活动的 Idle 句柄时，循环将执行零超时轮询，而不是为 I/O 而阻塞。

> **警告**：尽管名字如此，Idle 句柄的回调会在每次循环迭代时都被调用，而不是在循环真正"空闲"时。

## 数据类型

### `uv_idle_t`

Idle 句柄类型。

### `void (*uv_idle_cb)(uv_idle_t* handle)`

传递给 `uv_idle_start()` 的回调的类型定义。

### 公共成员

无。

> **参见**：`uv_handle_t` 的成员同样适用。

## API

### `int uv_idle_init(uv_loop_t* loop, uv_idle_t* idle)`

初始化句柄。此函数总是成功。

返回值：0。

### `int uv_idle_start(uv_idle_t* idle, uv_idle_cb cb)`

以给定的回调启动句柄。此函数总是成功，除非 `cb` 为 `NULL`。

返回值：成功时为 0，当 `cb == NULL` 时为 `UV_EINVAL`。

### `int uv_idle_stop(uv_idle_t* idle)`

停止句柄，回调将不再被调用。此函数总是成功。

返回值：0。

> **参见**：`uv_handle_t` 的 API 函数同样适用。

---

# 第五部分 `uv_async_t` — Async 句柄

Async 句柄允许用户"唤醒"事件循环，并让一个回调从另一个线程被调用。

> **注意**：对于给定的 async 句柄，`uv_async_send()` 与 `uv_async_cb` 的调用是顺序一致（seq_cst）的操作：在 `uv_async_send()` 之前进行的所有内存访问（读和写）对该回调均可见。

> **警告**：libuv 会合并（coalesce）对 `uv_async_send()` 的调用，也就是说，并非每一次调用都会导致回调执行一次。例如：如果在回调被调用之前连续调用了 5 次 `uv_async_send()`，回调将只被调用一次。如果在回调被调用之后再次调用 `uv_async_send()`，回调会再次被调用。不过，由于操作是顺序一致的，该回调中读到的或写入的值将始终与其他线程读到的或写入的值相同（或更新）。

> **注意**：1.53.0 版本变更：`uv_async_send()` 与 `uv_async_cb` 是顺序一致的。在此版本之前，libuv 可能合并调用的任何情形，出于正确性考虑很可能需要在其之前添加一个完整的 `seq_cst` 内存栅栏（fence）。

## 数据类型

### `uv_async_t`

Async 句柄类型。

### `void (*uv_async_cb)(uv_async_t* handle)`

传递给 `uv_async_init()` 的回调的类型定义。

### 公共成员

无。

> **参见**：`uv_handle_t` 的成员同样适用。

## API

### `int uv_async_init(uv_loop_t* loop, uv_async_t* async, uv_async_cb async_cb)`

初始化句柄。允许传入 NULL 回调。

返回值：成功时为 0，失败时为小于 0 的错误码。

> **注意**：与其他句柄初始化函数不同，它会立即启动该句柄。

### `int uv_async_send(uv_async_t* async)`

唤醒事件循环，并调用 async 句柄的回调。

返回值：成功时为 0，失败时为小于 0 的错误码。

> **注意**：可以安全地从任意线程调用此函数。回调将在循环线程中被调用。

> **注意**：`uv_async_send()` 是 [异步信号安全的（async-signal-safe）](https://man7.org/linux/man-pages/man7/signal-safety.7.html)。可以安全地在信号处理函数中调用此函数。

> **注意**：相对于使用同一个 async 句柄的其他调用和回调，此调用是一个完整的内存栅栏（memory fence），因此它会为发送线程和接收线程上围绕此调用及相应回调的所有操作建立顺序。

> **参见**：`uv_handle_t` 的 API 函数同样适用。
