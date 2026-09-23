# 文件系统操作（File system operations）

> 本章译自 libuv 官方文档 api/fs.rst（libuv 1.52）

libuv 提供了丰富的跨平台同步与异步文件系统操作。本文档中定义的所有函数都接受一个回调，且回调允许为 NULL。如果回调为 NULL，则请求以同步方式完成；否则将以异步方式执行。

所有文件操作都在线程池上运行。关于线程池大小的信息请参见线程池一章。

从 libuv v1.45.0 开始，Linux 上的一些文件操作在可能的情况下会被交给 [io_uring](https://en.wikipedia.org/wiki/Io_uring) 处理。除了吞吐量的（有时是显著的）提升之外，可观察到的行为不应有任何变化。当所需的内核特性不可用或不适用时，libuv 会回退到使用线程池。从 libuv v1.49.0 开始，该行为被回退，Linux 上的 libuv 默认重新使用线程池。要启用 io_uring，必须为 `uv_loop_t` 实例配置 `UV_LOOP_USE_IO_URING_SQPOLL` 选项。

> **注意**：在 Windows 上，`uv_fs_*` 函数使用 utf-8 编码。

## 数据类型

### `uv_fs_t`

文件系统请求类型。

### `uv_timespec_t`

用于存储具有纳秒精度的时间的数据类型，不兼容 Y2K38（2038 年问题）。将在 libuv v2.0 中被 `uv_timespec64_t` 取代。

```c
typedef struct {
    long tv_sec;
    long tv_nsec;
} uv_timespec_t;
```

### `uv_stat_t`

`struct stat` 的可移植等价物。

```c
typedef struct {
    uint64_t st_dev;
    uint64_t st_mode;
    uint64_t st_nlink;
    uint64_t st_uid;
    uint64_t st_gid;
    uint64_t st_rdev;
    uint64_t st_ino;
    uint64_t st_size;
    uint64_t st_blksize;
    uint64_t st_blocks;
    uint64_t st_flags;
    uint64_t st_gen;
    uv_timespec_t st_atim;
    uv_timespec_t st_mtim;
    uv_timespec_t st_ctim;
    uv_timespec_t st_birthtim;
} uv_stat_t;
```

### `uv_fs_type`

文件系统请求类型。

```c
typedef enum {
    UV_FS_UNKNOWN = -1,
    UV_FS_CUSTOM,
    UV_FS_OPEN,
    UV_FS_CLOSE,
    UV_FS_READ,
    UV_FS_WRITE,
    UV_FS_SENDFILE,
    UV_FS_STAT,
    UV_FS_LSTAT,
    UV_FS_FSTAT,
    UV_FS_FTRUNCATE,
    UV_FS_UTIME,
    UV_FS_FUTIME,
    UV_FS_ACCESS,
    UV_FS_CHMOD,
    UV_FS_FCHMOD,
    UV_FS_FSYNC,
    UV_FS_FDATASYNC,
    UV_FS_UNLINK,
    UV_FS_RMDIR,
    UV_FS_MKDIR,
    UV_FS_MKDTEMP,
    UV_FS_RENAME,
    UV_FS_SCANDIR,
    UV_FS_LINK,
    UV_FS_SYMLINK,
    UV_FS_READLINK,
    UV_FS_CHOWN,
    UV_FS_FCHOWN,
    UV_FS_REALPATH,
    UV_FS_COPYFILE,
    UV_FS_LCHOWN,
    UV_FS_OPENDIR,
    UV_FS_READDIR,
    UV_FS_CLOSEDIR,
    UV_FS_MKSTEMP,
    UV_FS_LUTIME
} uv_fs_type;
```

### `uv_statfs_t`

`struct statfs` 的精简跨平台等价物。用于 `uv_fs_statfs`。

```c
typedef struct uv_statfs_s {
    uint64_t f_type;
    uint64_t f_bsize;
    uint64_t f_blocks;
    uint64_t f_bfree;
    uint64_t f_bavail;
    uint64_t f_files;
    uint64_t f_ffree;
    uint64_t f_frsize;
    uint64_t f_spare[3];
} uv_statfs_t;
```

### `uv_dirent_type_t`

目录项（dirent）的类型。

```c
typedef enum {
    UV_DIRENT_UNKNOWN,
    UV_DIRENT_FILE,
    UV_DIRENT_DIR,
    UV_DIRENT_LINK,
    UV_DIRENT_FIFO,
    UV_DIRENT_SOCKET,
    UV_DIRENT_CHAR,
    UV_DIRENT_BLOCK
} uv_dirent_type_t;
```

### `uv_dirent_t`

`struct dirent` 的跨平台（精简）等价物。用于 `uv_fs_scandir_next`。

```c
typedef struct uv_dirent_s {
    const char* name;
    uv_dirent_type_t type;
} uv_dirent_t;
```

### `uv_dir_t`

用于流式目录遍历的数据类型。由 `uv_fs_opendir`、`uv_fs_readdir` 和 `uv_fs_closedir` 使用。`dirents` 表示用户提供的、用于存放结果的 `uv_dirent_t` 数组。`nentries` 是用户提供的 `dirents` 数组的最大长度。

```c
typedef struct uv_dir_s {
    uv_dirent_t* dirents;
    size_t nentries;
} uv_dir_t;
```

### `void (*uv_fs_cb)(uv_fs_t* req)`

请求异步完成时调用的回调。

### 公开成员

#### `uv_loop_t* uv_fs_t.loop`

启动该请求并在其中报告完成事件的循环。只读。

#### `uv_fs_type uv_fs_t.fs_type`

文件系统请求类型。

#### `const char* uv_fs_t.path`

请求所作用的路径。

#### `ssize_t uv_fs_t.result`

请求的结果。< 0 表示错误，否则表示成功。对于 `uv_fs_read` 或 `uv_fs_write` 等请求，该值分别表示读取或写入的数据量。

#### `uv_stat_t uv_fs_t.statbuf`

存储 `uv_fs_stat` 及其他 stat 请求的结果。

#### `void* uv_fs_t.ptr`

存储 `uv_fs_readlink` 和 `uv_fs_realpath` 的结果，并作为 `statbuf` 的别名。

> **参见**：`uv_req_t` 的成员同样适用。

## API

### `void uv_fs_req_cleanup(uv_fs_t* req)`

```c
void uv_fs_req_cleanup(uv_fs_t* req)
```

清理请求。必须在请求结束后调用，以释放 libuv 可能分配的内存。

### `int uv_fs_close(uv_loop_t* loop, uv_fs_t* req, uv_file file, uv_fs_cb cb)`

```c
int uv_fs_close(uv_loop_t* loop, uv_fs_t* req, uv_file file, uv_fs_cb cb)
```

等价于 `close(2)`。

### `int uv_fs_open(uv_loop_t* loop, uv_fs_t* req, const char* path, int flags, int mode, uv_fs_cb cb)`

```c
int uv_fs_open(uv_loop_t* loop, uv_fs_t* req, const char* path, int flags, int mode, uv_fs_cb cb)
```

等价于 `open(2)`。

> **注意**：在 Windows 上 libuv 使用 `CreateFileW`，因此文件始终以二进制模式打开。因此不支持 `O_BINARY` 和 `O_TEXT` 标志。

### `int uv_fs_read(uv_loop_t* loop, uv_fs_t* req, uv_file file, const uv_buf_t bufs[], unsigned int nbufs, int64_t offset, uv_fs_cb cb)`

```c
int uv_fs_read(uv_loop_t* loop, uv_fs_t* req, uv_file file, const uv_buf_t bufs[], unsigned int nbufs, int64_t offset, uv_fs_cb cb)
```

等价于 `preadv(2)`。如果 `offset` 参数为 `-1`，则使用并更新当前文件偏移量。

> **警告**：在 Windows 上，在非 MSVC 环境（例如使用 GCC 或 Clang 构建 libuv 时）下，使用 `UV_FS_O_FILEMAP` 打开的文件在内存映射读取操作失败时可能导致致命崩溃。

### `int uv_fs_unlink(uv_loop_t* loop, uv_fs_t* req, const char* path, uv_fs_cb cb)`

```c
int uv_fs_unlink(uv_loop_t* loop, uv_fs_t* req, const char* path, uv_fs_cb cb)
```

等价于 `unlink(2)`。

### `int uv_fs_write(uv_loop_t* loop, uv_fs_t* req, uv_file file, const uv_buf_t bufs[], unsigned int nbufs, int64_t offset, uv_fs_cb cb)`

```c
int uv_fs_write(uv_loop_t* loop, uv_fs_t* req, uv_file file, const uv_buf_t bufs[], unsigned int nbufs, int64_t offset, uv_fs_cb cb)
```

等价于 `pwritev(2)`。如果 `offset` 参数为 `-1`，则使用并更新当前文件偏移量。

> **警告**：在 Windows 上，在非 MSVC 环境（例如使用 GCC 或 Clang 构建 libuv 时）下，使用 `UV_FS_O_FILEMAP` 打开的文件在内存映射写入操作失败时可能导致致命崩溃。

### `int uv_fs_mkdir(uv_loop_t* loop, uv_fs_t* req, const char* path, int mode, uv_fs_cb cb)`

```c
int uv_fs_mkdir(uv_loop_t* loop, uv_fs_t* req, const char* path, int mode, uv_fs_cb cb)
```

等价于 `mkdir(2)`。

> **注意**：`mode` 目前在 Windows 上未实现。

### `int uv_fs_mkdtemp(uv_loop_t* loop, uv_fs_t* req, const char* tpl, uv_fs_cb cb)`

```c
int uv_fs_mkdtemp(uv_loop_t* loop, uv_fs_t* req, const char* tpl, uv_fs_cb cb)
```

等价于 `mkdtemp(3)`。结果可以在 `req->path` 处以 null 结尾字符串的形式找到。

### `int uv_fs_mkstemp(uv_loop_t* loop, uv_fs_t* req, const char* tpl, uv_fs_cb cb)`

```c
int uv_fs_mkstemp(uv_loop_t* loop, uv_fs_t* req, const char* tpl, uv_fs_cb cb)
```

等价于 `mkstemp(3)`。所创建文件的路径可以在 `req->path` 处以 null 结尾字符串的形式找到。文件描述符可以在 `req->result` 处以整数形式找到。

> 1.34.0 新增。

### `int uv_fs_rmdir(uv_loop_t* loop, uv_fs_t* req, const char* path, uv_fs_cb cb)`

```c
int uv_fs_rmdir(uv_loop_t* loop, uv_fs_t* req, const char* path, uv_fs_cb cb)
```

等价于 `rmdir(2)`。

### `int uv_fs_opendir(uv_loop_t* loop, uv_fs_t* req, const char* path, uv_fs_cb cb)`

```c
int uv_fs_opendir(uv_loop_t* loop, uv_fs_t* req, const char* path, uv_fs_cb cb)
```

将 `path` 作为目录流打开。成功时，会分配一个 `uv_dir_t` 并通过 `req->ptr` 返回。这块内存不会被 `uv_fs_req_cleanup()` 释放，但 `req->ptr` 会被置为 `NULL`。分配的内存必须通过调用 `uv_fs_closedir()` 来释放。失败时不分配任何内存。

可以通过将得到的 `uv_dir_t` 传给 `uv_fs_readdir()` 来遍历目录内容。

> 1.28.0 新增。

### `int uv_fs_closedir(uv_loop_t* loop, uv_fs_t* req, uv_dir_t* dir, uv_fs_cb cb)`

```c
int uv_fs_closedir(uv_loop_t* loop, uv_fs_t* req, uv_dir_t* dir, uv_fs_cb cb)
```

关闭 `dir` 所表示的目录流，并释放 `uv_fs_opendir()` 分配的内存。

> 1.28.0 新增。

### `int uv_fs_readdir(uv_loop_t* loop, uv_fs_t* req, uv_dir_t* dir, uv_fs_cb cb)`

```c
int uv_fs_readdir(uv_loop_t* loop, uv_fs_t* req, uv_dir_t* dir, uv_fs_cb cb)
```

遍历成功调用 `uv_fs_opendir()` 所返回的目录流 `dir`。在调用 `uv_fs_readdir()` 之前，调用者必须设置 `dir->dirents` 和 `dir->nentries`，分别表示用于存放读取到的目录项的 `uv_dirent_t` 元素数组及其大小。

成功时，结果是一个 >= 0 的整数，表示从流中读取的目录项数量。

> 1.28.0 新增。

> **警告**：`uv_fs_readdir()` 不是线程安全的。

> **注意**：此函数不会返回 "." 和 ".." 目录项。

> **注意**：成功时此函数会分配内存，必须使用 `uv_fs_req_cleanup()` 释放。`uv_fs_req_cleanup()` 必须在用 `uv_fs_closedir()` 关闭目录之前调用。

### `int uv_fs_scandir(uv_loop_t* loop, uv_fs_t* req, const char* path, int flags, uv_fs_cb cb)` / `int uv_fs_scandir_next(uv_fs_t* req, uv_dirent_t* ent)`

```c
int uv_fs_scandir(uv_loop_t* loop, uv_fs_t* req, const char* path, int flags, uv_fs_cb cb)
int uv_fs_scandir_next(uv_fs_t* req, uv_dirent_t* ent)
```

等价于 `scandir(3)`，但 API 略有不同。请求的回调被调用后，用户可以使用 `uv_fs_scandir_next` 填充 `ent`，获取下一个目录项数据。当没有更多目录项时，将返回 `UV_EOF`。

> **注意**：与 `scandir(3)` 不同，此函数不会返回 "." 和 ".." 目录项。

> **注意**：在 Linux 上，获取目录项类型只有部分文件系统支持（撰写本文时为 btrfs、ext2、ext3 和 ext4），请查阅 `getdents(2)` 手册页。

### `int uv_fs_stat(...)` / `int uv_fs_fstat(...)` / `int uv_fs_lstat(...)`

```c
int uv_fs_stat(uv_loop_t* loop, uv_fs_t* req, const char* path, uv_fs_cb cb)
int uv_fs_fstat(uv_loop_t* loop, uv_fs_t* req, uv_file file, uv_fs_cb cb)
int uv_fs_lstat(uv_loop_t* loop, uv_fs_t* req, const char* path, uv_fs_cb cb)
```

分别等价于 `stat(2)`、`fstat(2)` 和 `lstat(2)`。

### `int uv_fs_statfs(uv_loop_t* loop, uv_fs_t* req, const char* path, uv_fs_cb cb)`

```c
int uv_fs_statfs(uv_loop_t* loop, uv_fs_t* req, const char* path, uv_fs_cb cb)
```

等价于 `statfs(2)`。成功时，会分配一个 `uv_statfs_t` 并通过 `req->ptr` 返回。该内存由 `uv_fs_req_cleanup()` 释放。

> **注意**：结果 `uv_statfs_t` 中底层操作系统不支持的字段将被置为零。

> 1.31.0 新增。

### `int uv_fs_rename(uv_loop_t* loop, uv_fs_t* req, const char* path, const char* new_path, uv_fs_cb cb)`

```c
int uv_fs_rename(uv_loop_t* loop, uv_fs_t* req, const char* path, const char* new_path, uv_fs_cb cb)
```

等价于 `rename(2)`。

### `int uv_fs_fsync(uv_loop_t* loop, uv_fs_t* req, uv_file file, uv_fs_cb cb)`

```c
int uv_fs_fsync(uv_loop_t* loop, uv_fs_t* req, uv_file file, uv_fs_cb cb)
```

等价于 `fsync(2)`。

> **注意**：在 AIX 上，`uv_fs_fsync` 对引用非普通文件的文件描述符返回 `UV_EBADF`。

### `int uv_fs_fdatasync(uv_loop_t* loop, uv_fs_t* req, uv_file file, uv_fs_cb cb)`

```c
int uv_fs_fdatasync(uv_loop_t* loop, uv_fs_t* req, uv_file file, uv_fs_cb cb)
```

等价于 `fdatasync(2)`。

### `int uv_fs_ftruncate(uv_loop_t* loop, uv_fs_t* req, uv_file file, int64_t offset, uv_fs_cb cb)`

```c
int uv_fs_ftruncate(uv_loop_t* loop, uv_fs_t* req, uv_file file, int64_t offset, uv_fs_cb cb)
```

等价于 `ftruncate(2)`。

### `int uv_fs_copyfile(uv_loop_t* loop, uv_fs_t* req, const char* path, const char* new_path, int flags, uv_fs_cb cb)`

```c
int uv_fs_copyfile(uv_loop_t* loop, uv_fs_t* req, const char* path, const char* new_path, int flags, uv_fs_cb cb)
```

将文件从 `path` 复制到 `new_path`。支持的 `flags` 如下所述。

- `UV_FS_COPYFILE_EXCL`：如果存在此标志，当目标路径已存在时，`uv_fs_copyfile()` 将以 `UV_EEXIST` 失败。默认行为是目标存在时覆盖它。
- `UV_FS_COPYFILE_FICLONE`：如果存在此标志，`uv_fs_copyfile()` 将尝试创建写时复制（copy-on-write）反射链接（reflink）。如果底层平台不支持写时复制，或尝试使用写时复制时发生错误，则使用基于 `uv_fs_sendfile` 的回退复制机制。
- `UV_FS_COPYFILE_FICLONE_FORCE`：如果存在此标志，`uv_fs_copyfile()` 将尝试创建写时复制反射链接。如果底层平台不支持写时复制，或尝试使用写时复制时发生错误，则返回错误。

> **警告**：如果目标路径已创建，但复制数据时发生错误，则目标路径会被删除。在关闭和删除文件之间存在一个短暂的时间窗口，其他进程可能在此期间访问该文件。

> 1.14.0 新增。

> 1.20.0 变更：支持 `UV_FS_COPYFILE_FICLONE` 和 `UV_FS_COPYFILE_FICLONE_FORCE`。

> 1.33.0 变更：使用 `UV_FS_COPYFILE_FICLONE_FORCE` 时如果发生错误，将返回该错误。此前，所有错误都被映射为 `UV_ENOTSUP`。

### `int uv_fs_sendfile(uv_loop_t* loop, uv_fs_t* req, uv_file out_fd, uv_file in_fd, int64_t in_offset, size_t length, uv_fs_cb cb)`

```c
int uv_fs_sendfile(uv_loop_t* loop, uv_fs_t* req, uv_file out_fd, uv_file in_fd, int64_t in_offset, size_t length, uv_fs_cb cb)
```

功能受限的 `sendfile(2)` 等价物。

### `int uv_fs_access(uv_loop_t* loop, uv_fs_t* req, const char* path, int mode, uv_fs_cb cb)`

```c
int uv_fs_access(uv_loop_t* loop, uv_fs_t* req, const char* path, int mode, uv_fs_cb cb)
```

在 Unix 上等价于 `access(2)`。Windows 使用 `GetFileAttributesW()`。

### `int uv_fs_chmod(...)` / `int uv_fs_fchmod(...)`

```c
int uv_fs_chmod(uv_loop_t* loop, uv_fs_t* req, const char* path, int mode, uv_fs_cb cb)
int uv_fs_fchmod(uv_loop_t* loop, uv_fs_t* req, uv_file file, int mode, uv_fs_cb cb)
```

分别等价于 `chmod(2)` 和 `fchmod(2)`。

### `int uv_fs_utime(...)` / `int uv_fs_futime(...)` / `int uv_fs_lutime(...)`

```c
int uv_fs_utime(uv_loop_t* loop, uv_fs_t* req, const char* path, double atime, double mtime, uv_fs_cb cb)
int uv_fs_futime(uv_loop_t* loop, uv_fs_t* req, uv_file file, double atime, double mtime, uv_fs_cb cb)
int uv_fs_lutime(uv_loop_t* loop, uv_fs_t* req, const char* path, double atime, double mtime, uv_fs_cb cb)
```

分别等价于 `utime(2)`、`futimes(3)` 和 `lutimes(3)`。

将 `UV_FS_UTIME_NOW` 作为 atime 或 mtime 传入时，时间戳将被设置为当前时间。

将 `UV_FS_UTIME_OMIT` 作为 atime 或 mtime 传入时，时间戳保持不变。

> **注意**：z/OS：`uv_fs_lutime()` 未在 z/OS 上实现。仍然可以调用它，但会返回 `UV_ENOSYS`。

> **注意**：AIX：`uv_fs_futime()` 和 `uv_fs_lutime()` 函数仅适用于 AIX 7.1 及更新版本。在更旧的版本上仍然可以调用，但会返回 `UV_ENOSYS`。

> 1.10.0 变更：Windows 支持亚秒精度。

### `int uv_fs_link(uv_loop_t* loop, uv_fs_t* req, const char* path, const char* new_path, uv_fs_cb cb)`

```c
int uv_fs_link(uv_loop_t* loop, uv_fs_t* req, const char* path, const char* new_path, uv_fs_cb cb)
```

等价于 `link(2)`。

### `int uv_fs_symlink(uv_loop_t* loop, uv_fs_t* req, const char* path, const char* new_path, int flags, uv_fs_cb cb)`

```c
int uv_fs_symlink(uv_loop_t* loop, uv_fs_t* req, const char* path, const char* new_path, int flags, uv_fs_cb cb)
```

等价于 `symlink(2)`。

> **注意**：在 Windows 上，可以指定 `flags` 参数来控制符号链接的创建方式：
>
> - `UV_FS_SYMLINK_DIR`：指示 `path` 指向一个目录。
> - `UV_FS_SYMLINK_JUNCTION`：请求使用 junction point（联接点）创建符号链接。

### `int uv_fs_readlink(uv_loop_t* loop, uv_fs_t* req, const char* path, uv_fs_cb cb)`

```c
int uv_fs_readlink(uv_loop_t* loop, uv_fs_t* req, const char* path, uv_fs_cb cb)
```

等价于 `readlink(2)`。结果字符串存储在 `req->ptr` 中。

### `int uv_fs_realpath(uv_loop_t* loop, uv_fs_t* req, const char* path, uv_fs_cb cb)`

```c
int uv_fs_realpath(uv_loop_t* loop, uv_fs_t* req, const char* path, uv_fs_cb cb)
```

在 Unix 上等价于 `realpath(3)`。Windows 使用 [GetFinalPathNameByHandleW](https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-getfinalpathnamebyhandlew)。结果字符串存储在 `req->ptr` 中。

> **警告**：此函数存在一些在 Node 中使用时发现的平台特定注意事项。
>
> - macOS 及其他 BSD：如果在解析给定路径时发现超过 32 个符号链接，此函数将以 UV_ELOOP 失败。该限制是硬编码的，无法绕过。
> - Windows：虽然此函数在常见情况下可以工作，但存在一些无法处理的边角情况：
>   - 无法解析由绕过卷管理器的工具（如 ImDisk）创建的 ramdisk 卷中的路径。
>   - 使用盘符时大小写不一致。
>   - 解析出的路径会绕过 subst 映射的驱动器。
>
> 此函数仍然可以使用，但如果需要支持上述场景，则不推荐使用。
>
> 这些问题的背景故事和更多细节可以在[这里](https://github.com/nodejs/node/issues/7726)查看。

> 1.8.0 新增。

### `int uv_fs_chown(...)` / `int uv_fs_fchown(...)` / `int uv_fs_lchown(...)`

```c
int uv_fs_chown(uv_loop_t* loop, uv_fs_t* req, const char* path, uv_uid_t uid, uv_gid_t gid, uv_fs_cb cb)
int uv_fs_fchown(uv_loop_t* loop, uv_fs_t* req, uv_file file, uv_uid_t uid, uv_gid_t gid, uv_fs_cb cb)
int uv_fs_lchown(uv_loop_t* loop, uv_fs_t* req, const char* path, uv_uid_t uid, uv_gid_t gid, uv_fs_cb cb)
```

分别等价于 `chown(2)`、`fchown(2)` 和 `lchown(2)`。

> **注意**：这些函数在 Windows 上未实现。

> 1.21.0 变更：实现了 uv_fs_lchown。

### `uv_fs_type uv_fs_get_type(const uv_fs_t* req)`

```c
uv_fs_type uv_fs_get_type(const uv_fs_t* req)
```

返回 `req->fs_type`。

> 1.19.0 新增。

### `ssize_t uv_fs_get_result(const uv_fs_t* req)`

```c
ssize_t uv_fs_get_result(const uv_fs_t* req)
```

返回 `req->result`。

> 1.19.0 新增。

### `int uv_fs_get_system_error(const uv_fs_t* req)`

```c
int uv_fs_get_system_error(const uv_fs_t* req)
```

返回平台特定的错误代码——Windows 上为 `GetLastError()` 的值，其他平台上为 `-(req->result)`。

> 1.38.0 新增。

### `void* uv_fs_get_ptr(const uv_fs_t* req)`

```c
void* uv_fs_get_ptr(const uv_fs_t* req)
```

返回 `req->ptr`。

> 1.19.0 新增。

### `const char* uv_fs_get_path(const uv_fs_t* req)`

```c
const char* uv_fs_get_path(const uv_fs_t* req)
```

返回 `req->path`。

> 1.19.0 新增。

### `uv_stat_t* uv_fs_get_statbuf(uv_fs_t* req)`

```c
uv_stat_t* uv_fs_get_statbuf(uv_fs_t* req)
```

返回 `&req->statbuf`。

> 1.19.0 新增。

> **参见**：`uv_req_t` 的 API 函数同样适用。

## 辅助函数

### `uv_os_fd_t uv_get_osfhandle(int fd)`

```c
uv_os_fd_t uv_get_osfhandle(int fd)
```

对于 C 运行时中的文件描述符，获取其依赖操作系统的句柄。在 UNIX 上，原样返回 `fd`。在 Windows 上，此函数调用 [_get_osfhandle](https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/get-osfhandle?view=vs-2019)。注意返回值仍归 C 运行时所有，任何关闭它的尝试或在关闭 fd 之后使用它都可能导致故障。

> 1.12.0 新增。

### `int uv_open_osfhandle(uv_os_fd_t os_fd)`

```c
int uv_open_osfhandle(uv_os_fd_t os_fd)
```

对于依赖操作系统的句柄，获取其在 C 运行时中的文件描述符。在 UNIX 上，原样返回 `os_fd`。在 Windows 上，此函数调用 [_open_osfhandle](https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/open-osfhandle?view=vs-2019)。注意此函数会消耗该参数，任何关闭它的尝试或在关闭返回值之后使用它都可能导致故障。

> 1.23.0 新增。

## 文件打开常量

### `UV_FS_O_APPEND`

以追加模式打开文件。每次写入之前，文件偏移量都会定位到文件末尾。

### `UV_FS_O_CREAT`

如果文件不存在则创建它。

### `UV_FS_O_DIRECT`

文件 I/O 直接在用户空间缓冲区之间进行，缓冲区必须对齐。缓冲区大小和地址应是块设备物理扇区大小的倍数。

> **注意**：`UV_FS_O_DIRECT` 在 Linux 上受支持，在 Windows 上通过 [FILE_FLAG_NO_BUFFERING](https://learn.microsoft.com/en-us/windows/win32/fileio/file-buffering) 支持。`UV_FS_O_DIRECT` 在 macOS 上不受支持。

### `UV_FS_O_DIRECTORY`

如果路径不是目录，则打开失败。

> **注意**：`UV_FS_O_DIRECTORY` 在 Windows 上不受支持。

### `UV_FS_O_DSYNC`

以同步 I/O 方式打开文件。写入操作将在所有数据和最少量的元数据刷入磁盘后完成。

> **注意**：`UV_FS_O_DSYNC` 在 Windows 上通过 [FILE_FLAG_WRITE_THROUGH](https://learn.microsoft.com/en-us/windows/win32/fileio/file-buffering) 支持。

### `UV_FS_O_EXCL`

如果设置了 `O_CREAT` 标志且文件已存在，则打开失败。

> **注意**：一般而言，不与 `O_CREAT` 搭配使用时，`O_EXCL` 的行为是未定义的。有一个例外：在 Linux 2.6 及更高版本上，如果 pathname 指向块设备，`O_EXCL` 可以不与 `O_CREAT` 搭配使用。如果该块设备正被系统使用（例如已挂载），打开操作将以错误 `EBUSY` 失败。

### `UV_FS_O_EXLOCK`

以原子方式获取独占锁。

> **注意**：`UV_FS_O_EXLOCK` 仅在 macOS 和 Windows 上受支持。

> 1.17.0 变更：增加了对 Windows 的支持。

### `UV_FS_O_FILEMAP`

使用内存文件映射来访问文件。使用此标志时，文件不能被并发地多次打开。

> **注意**：`UV_FS_O_FILEMAP` 仅在 Windows 上受支持。

### `UV_FS_O_NOATIME`

读取文件时不更新文件访问时间。

> **注意**：`UV_FS_O_NOATIME` 在 Windows 上不受支持。

### `UV_FS_O_NOCTTY`

如果路径标识的是终端设备，打开该路径不会使该终端成为进程的控制终端（如果进程尚无控制终端）。

> **注意**：`UV_FS_O_NOCTTY` 在 Windows 上不受支持。

### `UV_FS_O_NOFOLLOW`

如果路径是符号链接，则打开失败。

> **注意**：`UV_FS_O_NOFOLLOW` 在 Windows 上不受支持。

### `UV_FS_O_NONBLOCK`

尽可能以非阻塞模式打开文件。

> **注意**：`UV_FS_O_NONBLOCK` 在 Windows 上不受支持。

### `UV_FS_O_RANDOM`

访问意图是随机的。系统可以将其作为优化文件缓存的提示。

> **注意**：`UV_FS_O_RANDOM` 仅在 Windows 上通过 [FILE_FLAG_RANDOM_ACCESS](https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createfilew) 支持。

### `UV_FS_O_RDONLY`

以只读方式打开文件。

### `UV_FS_O_RDWR`

以读写方式打开文件。

### `UV_FS_O_SEQUENTIAL`

访问意图是从头到尾的顺序读取。系统可以将其作为优化文件缓存的提示。

> **注意**：`UV_FS_O_SEQUENTIAL` 仅在 Windows 上通过 [FILE_FLAG_SEQUENTIAL_SCAN](https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createfilew) 支持。

### `UV_FS_O_SHORT_LIVED`

文件是临时文件，应尽可能不将其刷入磁盘。

> **注意**：`UV_FS_O_SHORT_LIVED` 仅在 Windows 上通过 [FILE_ATTRIBUTE_TEMPORARY](https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createfilew) 支持。

### `UV_FS_O_SYMLINK`

打开符号链接本身，而不是它所指向的资源。

### `UV_FS_O_SYNC`

以同步 I/O 方式打开文件。写入操作将在所有数据和所有元数据刷入磁盘后完成。

> **注意**：`UV_FS_O_SYNC` 在 Windows 上通过 [FILE_FLAG_WRITE_THROUGH](https://learn.microsoft.com/en-us/windows/win32/fileio/file-buffering) 支持。

### `UV_FS_O_TEMPORARY`

文件是临时文件，应尽可能不将其刷入磁盘。

> **注意**：`UV_FS_O_TEMPORARY` 仅在 Windows 上通过 [FILE_ATTRIBUTE_TEMPORARY](https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createfilew) 支持。

### `UV_FS_O_TRUNC`

如果文件存在且是普通文件，并且以写权限成功打开，则其长度将被截断为零。

### `UV_FS_O_WRONLY`

以只写方式打开文件。
