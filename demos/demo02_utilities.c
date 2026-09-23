/**
 * demo02 - 实用工具函数：高精度时间、内存信息、CPU 信息
 *
 * 编译: gcc demo02_utilities.c -o demo02 -I$HOME/.local/include -L$HOME/.local/lib -luv -Wl,-rpath,$HOME/.local/lib
 * 知识点: uv_hrtime / uv_uptime / uv_get_free_memory / uv_cpu_info / uv_version_string
 */
#include <stdio.h>
#include <uv.h>

int main(void) {
    /* 纳秒级单调时钟 —— 常用于性能打点 */
    uint64_t t0 = uv_hrtime();
    for (volatile int i = 0; i < 10000000; i++);  /* 模拟耗时操作 */
    uint64_t t1 = uv_hrtime();
    printf("循环耗时 %.2f ms\n", (t1 - t0) / 1e6);

    /* 系统级信息 */
    double uptime = 0;
    uv_uptime(&uptime);
    printf("系统已运行: %.1f 秒\n", uptime);
    printf("空闲内存: %.1f MB\n", uv_get_free_memory() / 1048576.0);
    printf("总内存:   %.1f MB\n", uv_get_total_memory() / 1048576.0);
    printf("本机处理器数: %d\n", uv_available_parallelism());

    /* CPU 信息列表（用完必须调 uv_free_cpu_info 释放） */
    uv_cpu_info_t *cpus;
    int count;
    uv_cpu_info(&cpus, &count);
    for (int i = 0; i < count && i < 4; i++) {
        printf("CPU[%d]: %s, %d MHz\n", i, cpus[i].model, (int)cpus[i].speed);
    }
    uv_free_cpu_info(cpus, count);

    /* libuv 版本（运行时探测，可用于兼容性检查） */
    printf("libuv 版本: %s (0x%x)\n", uv_version_string(), uv_version());
    return 0;
}
