/**
 * demo11 - 信号处理：Ctrl+C 优雅退出（清理资源再走）
 *
 * 编译: gcc demo11_signal.c -o demo11 -I$HOME/.local/include -L$HOME/.local/lib -luv -Wl,-rpath,$HOME/.local/lib
 * 知识点: uv_signal_init / uv_signal_start / SIGINT / uv_unref
 *
 * 生产场景：收到 SIGTERM/SIGINT 后停止 accept 新连接、
 * 刷缓冲、关数据库，然后再退出 —— 而不是被一棍子打死。
 */
#include <stdio.h>
#include <unistd.h>
#include <uv.h>

void on_sigint(uv_signal_t *handle, int signum) {
    printf("\n收到 SIGINT(%d), 开始优雅退出...\n", signum);
    /* 这里可以做清理：关文件、刷日志、通知下游等 */
    uv_signal_stop(handle);            /* 不再处理后续 SIGINT */
    uv_stop(uv_default_loop());        /* 停止事件循环 */
}

int main(void) {
    uv_loop_t *loop = uv_default_loop();

    uv_signal_t sig;
    uv_signal_init(loop, &sig);
    uv_signal_start(&sig, on_sigint, SIGINT);

    /* 业务句柄：一个每秒嘀嗒的心跳定时器 */
    uv_timer_t heartbeat;
    uv_timer_init(loop, &heartbeat);
    uv_timer_start(&heartbeat, NULL, 1000, 1000);   /* NULL 回调也占位 */
    printf("运行中... 按 Ctrl+C 触发优雅退出\n");

    uv_run(loop, UV_RUN_DEFAULT);

    printf("清理完成, 再见\n");
    uv_loop_close(loop);
    return 0;
}
