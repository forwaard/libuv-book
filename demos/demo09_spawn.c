/**
 * demo09 - 子进程：执行外部命令并捕获 stdout，用事件循环等待退出
 *
 * 编译: gcc demo09_spawn.c -o demo09 -I$HOME/.local/include -L$HOME/.local/lib -luv -Wl,-rpath,$HOME/.local/lib
 * 知识点: uv_spawn / uv_pipe_t 重定向 / uv_process_t / uv_expath / exit_cb
 *
 * 核心模式：把子进程的 stdout 接到一个 uv_pipe_t 上，
 * 然后对该管道 uv_read_start —— 子进程输出就像网络流一样异步到达。
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <uv.h>

uv_process_t child_req;
uv_process_options_t options;
uv_loop_t *loop;
uv_pipe_t child_stdout;

void alloc_buffer(uv_handle_t *h, size_t suggested, uv_buf_t *buf) {
    *buf = uv_buf_init((char*)malloc(suggested), suggested);
}

void on_child_exit(uv_process_t *req, int64_t exit_status, int term_signal) {
    fprintf(stderr, "[parent] 子进程退出, code=%ld, signal=%d\n",
            (long)exit_status, term_signal);
    uv_close((uv_handle_t*)req, NULL);
}

void on_read(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
    if (nread > 0) {
        printf("[child stdout] %.*s", (int)nread, buf->base);
    } else if (nread < 0) {
        /* 管道 EOF：子进程已关闭输出，顺带停止监听 */
        uv_read_stop(stream);
    }
    free(buf->base);
}

int main(void) {
    loop = uv_default_loop();

    uv_pipe_init(loop, &child_stdout, 0);

    /* 把子进程 stdout(1) 重定向到我们的管道；stderr 交给继承的 fd=2 */
    uv_stdio_container_t child_stdio[3];
    child_stdio[0].flags = UV_IGNORE;
    child_stdio[1].flags = UV_CREATE_PIPE | UV_WRITABLE_PIPE;
    child_stdio[1].data.stream = (uv_stream_t*)&child_stdout;
    child_stdio[2].flags = UV_INHERIT_FD;
    child_stdio[2].data.fd = 2;

    options.stdio_count = 3;
    options.stdio = child_stdio;
    options.exit_cb = on_child_exit;
    options.file = "ls";          /* 直接执行，不经过 shell */
    options.args = (char*[]){"ls", "-l", "/etc/hostname", NULL};

    int r;
    if ((r = uv_spawn(loop, &child_req, &options))) {
        fprintf(stderr, "spawn 失败: %s\n", uv_strerror(r));
        return 1;
    }
    fprintf(stderr, "[parent] 已启动子进程 pid=%ld\n", (long)child_req.pid);

    uv_read_start((uv_stream_t*)&child_stdout, alloc_buffer, on_read);

    uv_run(loop, UV_RUN_DEFAULT);
    uv_loop_close(loop);
    return 0;
}
