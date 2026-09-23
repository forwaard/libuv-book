/**
 * demo06 - UDP 发送与接收（数据报语义）
 *
 * 编译: gcc demo06_udp.c -o demo06 -I$HOME/.local/include -L$HOME/.local/lib -luv -Wl,-rpath,$HOME/.local/lib
 * 运行: ./demo06 receiver 挂后台, 然后 ./demo06 sender（见 run06.sh）
 * 知识点: uv_udp_init / uv_udp_bind / uv_udp_recv_start / uv_udp_send / uv_udp_set_membership
 *
 * UDP 是无连接数据报：没有 accept/connect 握手，recv_start 后直接收包。
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <uv.h>

#define PORT 7001
#define MULTICAST_GROUP "239.255.0.1"   /* 本地管理组播地址段 */

uv_udp_t recv_socket;
uv_udp_t send_socket;

void alloc_buffer(uv_handle_t *h, size_t suggested, uv_buf_t *buf) {
    *buf = uv_buf_init((char*)malloc(suggested), suggested);
}

void on_recv(uv_udp_t *handle, ssize_t nread, const uv_buf_t *buf,
             const struct sockaddr *addr, unsigned flags) {
    if (nread > 0 && addr != NULL) {
        char ip[INET_ADDRSTRLEN];
        uv_ip4_name((const struct sockaddr_in*)addr, ip, sizeof(ip));
        int port = ntohs(((const struct sockaddr_in*)addr)->sin_port);
        printf("[receiver] 收到来自 %s:%d 的 %ld 字节: %.*s",
               ip, port, (long)nread, (int)nread, buf->base);
    }
    free(buf->base);
    if (nread == 0 && addr == NULL) {
        /* addr==NULL 表示没有更多待处理数据报 */
        uv_udp_recv_stop(handle);
    }
}

void on_send(uv_udp_send_t *req, int status) {
    if (status) fprintf(stderr, "[sender] 发送失败: %s\n", uv_strerror(status));
    free(req);
    printf("[sender] 数据报已发送\n");
}

int main(int argc, char **argv) {
    uv_loop_t *loop = uv_default_loop();

    if (argc > 1 && strcmp(argv[1], "receiver") == 0) {
        uv_udp_init(loop, &recv_socket);
        struct sockaddr_in addr;
        uv_ip4_addr("0.0.0.0", PORT, &addr);
        uv_udp_bind(&recv_socket, (const struct sockaddr*)&addr, UV_UDP_REUSEADDR);
        /* 加入组播组（可选；单播场景不需要） */
        uv_udp_set_membership(&recv_socket, MULTICAST_GROUP, NULL, UV_JOIN_GROUP);
        uv_udp_recv_start(&recv_socket, alloc_buffer, on_recv);
        printf("[receiver] UDP 监听 0.0.0.0:%d (组播组 %s)\n", PORT, MULTICAST_GROUP);
    } else {
        uv_udp_init(loop, &send_socket);
        struct sockaddr_in dest;
        uv_ip4_addr(MULTICAST_GROUP, PORT, &dest);
        const char *msg = "UDP 组播数据报\n";
        uv_buf_t buf = uv_buf_init((char*)msg, strlen(msg));
        uv_udp_send_t *req = malloc(sizeof(uv_udp_send_t));
        uv_udp_send(req, &send_socket, &buf, 1, (const struct sockaddr*)&dest, on_send);
    }
    uv_run(loop, UV_RUN_DEFAULT);
    uv_loop_close(loop);
    return 0;
}
