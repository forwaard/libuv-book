/**
 * demo04 - 定时器进阶 + prepare/check 钩子：事件循环每一圈的完整生命周期
 *
 * 编译: gcc demo04_queue_watch.c -o demo04 -I$HOME/.local/include -L$HOME/.local/lib -luv -Wl,-rpath,$HOME/.local/lib
 * 知识点: uv_prepare_t(每圈前) / uv_check_t(每圈后) / uv_timer_t 周期模式 / uv_unref
 *
 * 事件循环一圈的顺序: timers -> pending -> prepare(圈前钩子)
 *                     -> poll I/O -> check(圈后钩子) -> 下一圈
 */
#include <stdio.h>
#include <uv.h>

static int ticks = 0;

void on_prepare(uv_prepare_t *h) {  /* 每圈事件处理之前 */
    printf("[prepare] 第 %d 圈开始\n", ticks + 1);
}
void on_check(uv_check_t *h) {      /* 每圈事件处理之后 */
    printf("[check]   第 %d 圈结束\n", ticks + 1);
}
void on_timer(uv_timer_t *h) {      /* 每 250ms 一次，repeat 模式 */
    ticks++;
    printf("[timer]   嘀嗒 %d\n", ticks);
    if (ticks >= 3) {
        printf("3 次到, 停止计时器\n");
        uv_timer_stop(h);
        /* 手动停止循环（因为 prepare/check 不阻止循环退出，
           但为了演示 uv_stop 的用法这里显式调用） */
        uv_stop(uv_default_loop());
    }
}

int main(void) {
    uv_loop_t *loop = uv_default_loop();

    uv_prepare_t prep;
    uv_prepare_init(loop, &prep);
    uv_prepare_start(&prep, on_prepare);

    uv_check_t chk;
    uv_check_init(loop, &chk);
    uv_check_start(&chk, on_check);

    uv_timer_t tick;
    uv_timer_init(loop, &tick);
    uv_timer_start(&tick, on_timer, 250, 250);  /* 首次 250ms，之后每 250ms */

    uv_run(loop, UV_RUN_DEFAULT);

    /* 清理所有句柄后才能成功 uv_loop_close */
    uv_prepare_stop(&prep); uv_check_stop(&chk); uv_timer_stop(&tick);
    uv_close((uv_handle_t*)&prep, NULL);
    uv_close((uv_handle_t*)&chk, NULL);
    uv_close((uv_handle_t*)&tick, NULL);
    uv_run(loop, UV_RUN_NOWAIT);   /* 跑一圈让 close 回调执行 */
    uv_loop_close(loop);
    printf("干净退出\n");
    return 0;
}
