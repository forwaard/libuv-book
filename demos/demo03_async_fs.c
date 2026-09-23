/**
 * demo03 - 异步文件系统操作：回调式读写，体会"文件 I/O 走线程池"
 *
 * 编译: gcc demo03_async_fs.c -o demo03 -I$HOME/.local/include -L$HOME/.local/lib -luv -Wl,-rpath,$HOME/.local/lib
 * 知识点: uv_fs_open / uv_fs_read / uv_fs_write / uv_fs_close / uv_fs_cb
 *
 * 关键认知: libuv 的文件操作是【同步系统调用包装成异步】，实际由线程池里的
 * 工作线程执行，完成后回调在主线程（事件循环线程）执行 —— 所以回调里可以
 * 安全地继续操作事件循环。
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <uv.h>

uv_fs_t open_req, read_req, write_req, close_req;
static char buffer[1024];

void on_read(uv_fs_t *req);
void on_open(uv_fs_t *req) {
    if (req->result < 0) {
        fprintf(stderr, "打开失败: %s\n", uv_strerror((int)req->result));
        return;
    }
    printf("文件已打开, fd=%ld\n", (long)req->result);
    /* 从 fd 起点读 1024 字节（uv_buf_init 构造缓冲区） */
    uv_buf_t iov = uv_buf_init(buffer, sizeof(buffer));
    uv_fs_read(uv_default_loop(), &read_req, req->result, &iov, 1, 0, on_read);
}

void on_read(uv_fs_t *req) {
    if (req->result < 0) {
        fprintf(stderr, "读取失败: %s\n", uv_strerror((int)req->result));
        return;
    }
    if (req->result == 0) {            /* 读到 EOF */
        uv_fs_close(uv_default_loop(), &close_req, open_req.result, NULL);
        uv_fs_req_cleanup(req);
        return;
    }
    printf("读到 %ld 字节:\n---\n%.*s---\n", (long)req->result, (int)req->result, buffer);
    uv_fs_req_cleanup(req);
    uv_fs_close(uv_default_loop(), &close_req, open_req.result, NULL);
}

int main(int argc, char **argv) {
    const char *path = argc > 1 ? argv[1] : "/etc/hostname";
    /* 最后一个参数是回调 NULL 表示同步模式，这里传 on_open 走异步 */
    uv_fs_open(uv_default_loop(), &open_req, path, O_RDONLY, 0, on_open);
    uv_run(uv_default_loop(), UV_RUN_DEFAULT);
    uv_fs_req_cleanup(&open_req);
    uv_fs_req_cleanup(&close_req);
    uv_loop_close(uv_default_loop());
    return 0;
}
