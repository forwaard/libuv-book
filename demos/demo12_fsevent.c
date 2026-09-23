/**
 * demo12 - 文件系统事件监控（fs_event）：目录变更实时感知
 *
 * 编译: gcc demo12_fsevent.c -o demo12 -I$HOME/.local/include -L$HOME/.local/lib -luv -Wl,-rpath,$HOME/.local/lib
 * 运行: 见 run12.sh（后台监控 demo_dir，然后往里写文件触发事件）
 * 知识点: uv_fs_event_init / uv_fs_event_start / UV_RENAME / UV_CHANGE
 *
 * 底层: Linux 用 inotify，macOS 用 FSEvents，Windows 用 ReadDirectoryChangesW
 * —— libuv 把三家差异抹平成同一套回调接口。
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <uv.h>

void on_fs_event(uv_fs_event_t *handle, const char *filename, int events, int status) {
    if (status < 0) {
        fprintf(stderr, "监控错误: %s\n", uv_strerror(status));
        return;
    }
    const char *type = (events & UV_RENAME) ? "重命名/增删" :
                       (events & UV_CHANGE) ? "内容修改" : "其他";
    printf("[fs_event] 文件 %s 发生: %s\n", filename ? filename : "?", type);
}

void on_timeout(uv_timer_t *t) {
    printf("监控 5 秒结束, 退出\n");
    uv_stop(uv_default_loop());
}

int main(int argc, char **argv) {
    const char *dir = argc > 1 ? argv[1] : ".";
    uv_loop_t *loop = uv_default_loop();

    uv_fs_event_t *watcher = malloc(sizeof(uv_fs_event_t));
    uv_fs_event_init(loop, watcher);
    int r = uv_fs_event_start(watcher, on_fs_event, dir, 0);
    if (r) {
        fprintf(stderr, "启动监控失败: %s\n", uv_strerror(r));
        return 1;
    }
    printf("正在监控目录: %s\n", dir);

    uv_timer_t t;
    uv_timer_init(loop, &t);
    uv_timer_start(&t, on_timeout, 5000, 0);   /* 5 秒后自动结束 */

    uv_run(loop, UV_RUN_DEFAULT);
    uv_fs_event_stop(watcher);
    uv_close((uv_handle_t*)watcher, NULL);
    free(watcher);
    uv_loop_close(loop);
    return 0;
}
