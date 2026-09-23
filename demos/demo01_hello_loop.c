/**
 * demo01 - 第一个 libuv 程序：事件循环 + idle 句柄 + 定时器
 *
 * 编译: gcc demo01_hello_loop.c -o demo01 -I$HOME/.local/include -L$HOME/.local/lib -luv -Wl,-rpath,$HOME/.local/lib
 * 知识点: uv_loop_init / uv_idle_t / uv_timer_t / uv_run / uv_stop
 */
#include <stdio.h>
#include <uv.h>

int64_t counter = 0;

/* idle 回调：事件循环每转一圈，没有其他事件可处理时就调用它 */
void idle_cb(uv_idle_t *handle) {
    counter++;
    if (counter >= 100000) {
        /* 停掉 idle 句柄（引用计数减一），事件循环才能退出 */
        uv_idle_stop(handle);
    }
}

/* 定时器回调：100ms 后触发一次 */
void timer_cb(uv_timer_t *handle) {
    printf("100ms 定时器触发, idle 已执行 %ld 次\n", counter);
    /* 让事件循环自然退出：所有句柄都停止后 uv_run 返回 */
}

int main(void) {
    uv_loop_t *loop = uv_default_loop();

    uv_idle_t idler;
    uv_idle_init(loop, &idler);
    uv_idle_start(&idler, idle_cb);

    uv_timer_t timer;
    uv_timer_init(loop, &timer);
    uv_timer_start(&timer, timer_cb, 100, 0);  /* 100ms 后触发，0 = 不重复 */

    printf("开始运行事件循环...\n");
    uv_run(loop, UV_RUN_DEFAULT);

    uv_loop_close(loop);
    printf("事件循环退出\n");
    return 0;
}
