/**
 * demo13 - uv_queue_work：显式把任务丢进线程池（guide "threads" 章同款模式）
 *
 * 编译: gcc demo13_queue_work.c -o demo13 -I$HOME/.local/include -L$HOME/.local/lib -luv -Wl,-rpath,$HOME/.local/lib
 * 知识点: uv_queue_work / uv_work_t / uv_after_work_cb / after_work_cb
 *
 * 这是 libuv 提供的"我不想阻塞事件循环"的标准答案：
 * 把 CPU 密集或阻塞任务打包成 work，线程池执行，结果回到主线程回调。
 * 文件 I/O 和 DNS 解析内部用的就是同一套机制。
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <uv.h>

/* 传给工作线程的上下文（顺便演示 after_work 里怎么拿到它） */
typedef struct {
    int start;
    int end;
    long result;
} task_t;

/* 【线程池线程】执行阻塞/耗时任务 —— 这里可以是任何阻塞调用 */
void do_work(uv_work_t *req) {
    task_t *task = (task_t*)req->data;
    printf("[pool]   开始计算 %d..%d\n", task->start, task->end);
    sleep(1);                          /* 模拟耗时（或阻塞 I/O） */
    for (int i = task->start; i <= task->end; i++) task->result += i;
}

/* 【主线程】工作完成后的回调 —— 此时结果已经就绪 */
void after_work(uv_work_t *req, int status) {
    if (status < 0) {
        fprintf(stderr, "[main]   任务失败: %s\n", uv_strerror(status));
        free(req->data);
        free(req);
        return;
    }
    task_t *task = (task_t*)req->data;
    printf("[main]   任务完成: %d..%d 累加 = %ld\n",
           task->start, task->end, task->result);
    free(task);
    free(req);
    /* 所有 work 完成后事件循环自然退出 */
}

int main(void) {
    uv_loop_t *loop = uv_default_loop();

    /* 派发两个并行任务（默认线程池 4 线程，可真并行） */
    int ranges[2][2] = {{1, 1000000}, {2000000, 3000000}};
    for (int i = 0; i < 2; i++) {
        uv_work_t *req = malloc(sizeof(uv_work_t));
        task_t *task = malloc(sizeof(task_t));
        task->start = ranges[i][0];
        task->end = ranges[i][1];
        task->result = 0;
        req->data = task;
        uv_queue_work(loop, req, do_work, after_work);
    }
    printf("[main]   已派发 2 个任务到线程池, 主线程继续转事件循环\n");

    uv_run(loop, UV_RUN_DEFAULT);
    uv_loop_close(loop);
    printf("[main]   全部完成\n");
    return 0;
}
