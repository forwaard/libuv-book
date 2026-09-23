/**
 * demo10 - 多线程 + uv_async_t：工作线程如何安全地"唤醒"事件循环
 *
 * 编译: gcc demo10_threads.c -o demo10 -I$HOME/.local/include -L$HOME/.local/lib -luv -Wl,-rpath,$HOME/.local/lib
 * 知识点: uv_thread_create / uv_mutex_t / uv_async_init / uv_async_send
 *
 * 【libuv 最重要的并发铁律】事件循环和它的回调只能在创建它的那个线程跑。
 * 工作线程想和事件循环通信，唯一安全的方式就是 uv_async_send ——
 * 它会"从旁边捅一下"事件循环，让 async 回调在主线程执行。
 * 共享数据必须用锁保护。
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <uv.h>

uv_async_t async;          /* 跨线程唤醒句柄 */
uv_mutex_t lock;           /* 保护共享结果的互斥锁 */
long computed_result = 0;  /* 共享数据：工作线程写，主线程读 */
int done = 0;              /* 完成标志 */

/* 工作线程：模拟耗时计算（比如解码一个文件） */
void worker(void *arg) {
    long sum = 0;
    for (int i = 1; i <= 1000000; i++) {
        sum += i;
    }
    /* 加锁写共享数据 —— 任何跨线程数据都要有锁 */
    uv_mutex_lock(&lock);
    computed_result = sum;
    done = 1;
    uv_mutex_unlock(&lock);

    /* 唤醒事件循环：async 回调将在主线程执行 */
    uv_async_send(&async);
    printf("[worker]  计算完成, 已发 async 唤醒\n");
}

/* async 回调：运行在主线程（事件循环线程），可以安全操作任何句柄 */
void on_async(uv_async_t *handle) {
    uv_mutex_lock(&lock);
    if (done) {
        printf("[main]    收到唤醒, 结果 = %ld\n", computed_result);
        done = 0;
        /* 读完可以关掉 async 句柄，减少事件循环的监听负担 */
        uv_close((uv_handle_t*)handle, NULL);
    }
    uv_mutex_unlock(&lock);
}

int main(void) {
    uv_loop_t *loop = uv_default_loop();

    uv_async_init(loop, &async, on_async);   /* async 句柄让循环保持活跃 */
    uv_mutex_init(&lock);

    uv_thread_t tid;
    uv_thread_create(&tid, worker, NULL);
    printf("[main]    工作线程已启动 (tid=%lu)\n", (unsigned long)tid);

    uv_run(loop, UV_RUN_DEFAULT);   /* 主线程进入事件循环等待唤醒 */

    uv_thread_join(&tid);
    uv_mutex_destroy(&lock);
    uv_loop_close(loop);
    return 0;
}
