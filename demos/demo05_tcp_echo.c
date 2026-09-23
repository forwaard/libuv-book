/**
 * demo05 - TCP echo 服务器 + 客户端（单文件，用命令行参数区分）
 *
 * 编译: gcc demo05_tcp_echo.c -o demo05 -I$HOME/.local/include -L$HOME/.local/lib -luv -Wl,-rpath,$HOME/.local/lib
 * 运行: 终端A: ./demo05 server    终端B: ./demo05 client
 * 知识点: uv_tcp_init / uv_tcp_bind / uv_listen / uv_connection_cb
 *         uv_tcp_connect / uv_connect_cb / uv_read_start / uv_write
 *
 * 核心模式：server 监听后每来一个连接就 malloc 一个 client 句柄
 * （缓冲区指针挂在 handle->data 上），连接关闭时 free —— 这是 libuv TCP
 * 服务的标准"每连接一句柄"写法。
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <uv.h>

#define PORT 7000
#define BACKLOG 128

static uv_tcp_t server;
static int client_count = 0;

void alloc_buffer(uv_handle_t *h, size_t suggested, uv_buf_t *buf) {
    *buf = uv_buf_init((char*)malloc(suggested), suggested);
}

void on_close(uv_handle_t *h) {
    client_count--;
    printf("[server] 客户端断开, 当前在线 %d\n", client_count);
    free(h);
}

void on_write(uv_write_t *req, int status) {
    if (status < 0) fprintf(stderr, "[server] 写失败: %s\n", uv_strerror(status));
    free(req->data);   /* 我们把 buf.base 存在了 req->data */
    free(req);
}

void on_read(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
    if (nread < 0) {
        if (nread != UV_EOF) fprintf(stderr, "[server] 读错误: %s\n", uv_err_name(nread));
        uv_close((uv_handle_t*)stream, on_close);
    } else if (nread > 0) {
        printf("[server] 收到 %ld 字节: %.*s", (long)nread, (int)nread, buf->base);
        uv_write_t *req = malloc(sizeof(uv_write_t));
        req->data = buf->base;      /* write 完成回调里释放 */
        uv_write(req, stream, buf, 1, on_write);
        return;                     /* buf 所有权交给 on_write */
    }
    free(buf->base);                /* 没有转交所有权时必须释放 */
}

void on_new_connection(uv_stream_t *srv, int status) {
    if (status < 0) {
        fprintf(stderr, "[server] 新连接错误: %s\n", uv_strerror(status));
        return;
    }
    uv_tcp_t *client = malloc(sizeof(uv_tcp_t));
    uv_tcp_init(srv->loop, &(*client));
    if (uv_accept(srv, (uv_stream_t*)client) == 0) {
        client_count++;
        printf("[server] 新客户端接入, 当前在线 %d\n", client_count);
        uv_read_start((uv_stream_t*)client, alloc_buffer, on_read);
    } else {
        uv_close((uv_handle_t*)client, NULL);
    }
}

/* ------- 客户端部分 ------- */
void on_client_write(uv_write_t *req, int status) {
    if (status < 0) fprintf(stderr, "[client] 写失败: %s\n", uv_strerror(status));
    free(req);
}

void on_client_read(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
    if (nread > 0) {
        printf("[client] 收到回显: %.*s", (int)nread, buf->base);
        uv_read_stop((uv_stream_t*)stream);
        uv_close((uv_handle_t*)stream, NULL);   /* 拿到回显就退出 */
    } else if (nread < 0) {
        uv_close((uv_handle_t*)stream, NULL);
    }
    free(buf->base);
}

void on_connect(uv_connect_t *req, int status) {
    if (status < 0) {
        fprintf(stderr, "[client] 连接失败: %s\n", uv_strerror(status));
        exit(1);
    }
    const char *msg = "你好, libuv TCP!\n";
    uv_buf_t buf = uv_buf_init((char*)msg, strlen(msg));
    uv_write_t *wreq = malloc(sizeof(uv_write_t));
    uv_write(wreq, req->handle, &buf, 1, on_client_write);
    uv_read_start(req->handle, alloc_buffer, on_client_read);
    free(req);
}

int main(int argc, char **argv) {
    uv_loop_t *loop = uv_default_loop();
    if (argc > 1 && strcmp(argv[1], "client") == 0) {
        uv_tcp_t sock;
        uv_tcp_init(loop, &sock);
        uv_connect_t *req = malloc(sizeof(uv_connect_t));
        struct sockaddr_in dest;
        uv_ip4_addr("127.0.0.1", PORT, &dest);
        uv_tcp_connect(req, &sock, (const struct sockaddr*)&dest, on_connect);
    } else {
        uv_tcp_init(loop, &server);
        struct sockaddr_in addr;
        uv_ip4_addr("0.0.0.0", PORT, &addr);
        uv_tcp_bind(&server, (const struct sockaddr*)&addr, 0);
        int r = uv_listen((uv_stream_t*)&server, BACKLOG, on_new_connection);
        if (r) { fprintf(stderr, "监听失败: %s\n", uv_strerror(r)); return 1; }
        printf("[server] 正在监听 0.0.0.0:%d\n", PORT);
    }
    uv_run(loop, UV_RUN_DEFAULT);
    uv_loop_close(loop);
    return 0;
}
