/**
 * demo07 - 异步 DNS 解析 + 带 UV_THREADPOOL_SIZE 说明
 *
 * 编译: gcc demo07_dns.c -o demo07 -I$HOME/.local/include -L$HOME/.local/lib -luv -Wl,-rpath,$HOME/.local/lib
 * 知识点: uv_getaddrinfo / uv_freeaddrinfo / addrinfo 链表遍历
 *
 * DNS 解析也是走线程池的（因为 getaddrinfo 是阻塞调用），
 * 和文件 I/O 共享同一个线程池，默认 4 个线程，可用环境变量
 * UV_THREADPOOL_SIZE 调整。
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <uv.h>

void on_resolved(uv_getaddrinfo_t *resolver, int status, struct addrinfo *res) {
    if (status < 0) {
        fprintf(stderr, "解析失败: %s\n", uv_err_name(status));
        return;
    }
    char addr[64] = {'\0'};
    struct addrinfo *rp;
    int ipv4_count = 0;
    for (rp = res; rp != NULL; rp = rp->ai_next) {
        if (rp->ai_family == AF_INET) {           /* 只打印 IPv4 */
            uv_ip4_name((struct sockaddr_in*)rp->ai_addr, addr, sizeof(addr));
            printf("IPv4: %s\n", addr);
            ipv4_count++;
            if (ipv4_count >= 3) break;           /* 最多展示 3 条 */
        }
    }
    uv_freeaddrinfo(res);   /* 必须释放 addrinfo 链表 */
}

int main(int argc, char **argv) {
    const char *hostname = argc > 1 ? argv[1] : "example.com";
    struct addrinfo hints;
    hints.ai_family = AF_INET;          /* 只要 IPv4，可以改 AF_UNSPEC */
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = 0;

    uv_getaddrinfo_t resolver;
    printf("解析 %s ...\n", hostname);
    int r = uv_getaddrinfo(uv_default_loop(), &resolver, on_resolved,
                           hostname, NULL, &hints);
    if (r) {
        fprintf(stderr, "getaddrinfo 调用错误: %s\n", uv_err_name(r));
        return 1;
    }
    uv_run(uv_default_loop(), UV_RUN_DEFAULT);
    uv_loop_close(uv_default_loop());
    return 0;
}
