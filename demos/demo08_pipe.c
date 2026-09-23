/**
 * demo08 - 命名管道（Unix domain socket）服务端 + 客户端
 *
 * 编译: gcc demo08_pipe.c -o demo08 -I$HOME/.local/include -L$HOME/.local/lib -luv -Wl,-rpath,$HOME/.local/lib
 * 运行: 见 run08.sh
 * 知识点: uv_pipe_init / uv_pipe_bind / uv_pipe_connect / uv_write
 *
 * 管道类型是 uv_stream_t 的子类型，读写接口与 TCP 完全一致 ——
 * 这就是 libuv 抽象的力量：stream 层同一套 read/write。
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <uv.h>

#define SOCK_PATH "/tmp/libuv_demo08.sock"

void alloc_buffer(uv_handle_t *h, size_t suggested, uv_buf_t *buf) {
    *buf = uv_buf_init((char*)malloc(suggested), suggested);
}

/* ---------- 服务端 ---------- */
void on_srv_read(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
    if (nread > 0) {
        printf("[server] 收到: %.*s", (int)nread, buf->base);
        /* 回一句确认 */
        const char *ack = "server ACK\n";
        uv_buf_t out = uv_buf_init((char*)ack, strlen(ack));
        uv_write_t *wreq = malloc(sizeof(uv_write_t));
        uv_write(wreq, stream, &out, 1, NULL);
        uv_read_stop(stream);
        uv_close((uv_handle_t*)stream, NULL);  /* 处理完即断开该连接 */
    } else if (nread < 0) {
        uv_close((uv_handle_t*)stream, NULL);
    }
    free(buf->base);
}

void on_new_conn(uv_stream_t *srv, int status) {
    if (status < 0) return;
    uv_pipe_t *client = malloc(sizeof(uv_pipe_t));
    uv_pipe_init(srv->loop, client, 0);
    if (uv_accept(srv, (uv_stream_t*)client) == 0) {
        printf("[server] 管道客户端已接入\n");
        uv_read_start((uv_stream_t*)client, alloc_buffer, on_srv_read);
    } else {
        uv_close((uv_handle_t*)client, NULL);
    }
}

/* ---------- 客户端 ---------- */
void on_cli_read(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
    if (nread > 0) {
        printf("[client] 收到: %.*s", (int)nread, buf->base);
        uv_close((uv_handle_t*)stream, NULL);
    } else if (nread < 0) {
        uv_close((uv_handle_t*)stream, NULL);
    }
    free(buf->base);
}

void on_cli_write(uv_write_t *req, int status) {
    free(req);
    if (status) fprintf(stderr, "[client] 写失败: %s\n", uv_strerror(status));
}

void on_connect(uv_connect_t *req, int status) {
    if (status < 0) {
        fprintf(stderr, "[client] 连接失败: %s\n", uv_strerror(status));
        exit(1);
    }
    const char *msg = "hello via unix pipe\n";
    uv_buf_t buf = uv_buf_init((char*)msg, strlen(msg));
    uv_write_t *wreq = malloc(sizeof(uv_write_t));
    uv_write(wreq, req->handle, &buf, 1, on_cli_write);
    uv_read_start(req->handle, alloc_buffer, on_cli_read);
    free(req);
}

int main(int argc, char **argv) {
    uv_loop_t *loop = uv_default_loop();
    if (argc > 1 && strcmp(argv[1], "client") == 0) {
        uv_pipe_t client;
        uv_pipe_init(loop, &client, 0);
        uv_connect_t *req = malloc(sizeof(uv_connect_t));
        uv_pipe_connect(req, &client, SOCK_PATH, on_connect);
    } else {
        unlink(SOCK_PATH);   /* 清理残留 socket 文件 */
        uv_pipe_t srv;
        uv_pipe_init(loop, &srv, 0);
        int r = uv_pipe_bind(&srv, SOCK_PATH);
        if (r) { fprintf(stderr, "bind 失败: %s\n", uv_strerror(r)); return 1; }
        uv_listen((uv_stream_t*)&srv, 8, on_new_conn);
        printf("[server] 管道监听 %s\n", SOCK_PATH);
        uv_run(loop, UV_RUN_DEFAULT);
        unlink(SOCK_PATH);
        return 0;
    }
    uv_run(loop, UV_RUN_DEFAULT);
    uv_loop_close(loop);
    return 0;
}
