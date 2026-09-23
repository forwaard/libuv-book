# libuv 中文使用手册

> **官方文档全译 · 实战演练 13 讲** —— 基于 [libuv 1.52.2 官方文档](https://github.com/libuv/libuv)（commit `b05973e`）的完整中文翻译，附带一套由浅入深、全部经编译运行验证的实战示例。
>
> 📕 **成品 PDF（237 页，平板阅读优化）**：[`libuv-中文使用手册.pdf`](libuv-中文使用手册.pdf)

**作者**：forwaard <forwaard@163.com> · **仓库**：<https://github.com/forwaard/libuv-book>
**协议**：CC BY 4.0（与上游 libuv 文档许可一致，见 [LICENSE](LICENSE)）
[![License: CC BY 4.0](https://img.shields.io/badge/License-CC_BY_4.0-lightgrey.svg)](https://creativecommons.org/licenses/by/4.0/)

---

## 为什么做这个

libuv 是 Node.js 的事件循环底座，但官方文档只有英文，且缺少"从零跑起来"的完整路径。本仓库做了两件事：

1. **全译**：官方 31 篇文档（Guide 9 篇 + API 22 篇）无一遗漏译成中文，API 条目经脚本逐条核对（55 + 62 + 371 条，零缺失）。
2. **实战**：13 个独立完整的 C 示例，按学习曲线编排，每个都在 Linux + GCC 环境实际编译运行过，输出即文档。

## 目录

| 章节 | 内容 | 对应源文档 |
|------|------|-----------|
| 前言 / 实战导学 | 翻译原则、13 个 demo 的学习地图 | — |
| 01 libuv 基础 | 事件循环直觉、Hello World、默认循环 | guide/basics |
| 02 高级事件循环 | `uv_stop` 语义、`uv_run` 源码逐行讲解 | guide/eventloops |
| 03 实用工具库 | 定时器、idle/prepare/check、动态库插件系统 | guide/utilities |
| 04 文件系统 | 异步文件 I/O、缓冲区、线程池原理 | guide/filesystem |
| 05 网络编程 | TCP/UDP/DNS/网络接口查询 | guide/networking |
| 06 进程 | 子进程、IPC、信号、 detachment | guide/processes |
| 07 线程与同步原语 | 互斥锁/读写锁/条件变量/屏障 + `uv_queue_work` | guide/threads |
| 08–17 API 参考 | 事件循环、句柄与请求、定时器与异步唤醒、文件系统、文件监控、DNS 与线程池、TCP/UDP、流/管道/轮询/TTY、信号/进程/线程、杂项/错误码/设计概览 | api/*.rst 全部 22 篇 |

每个 md 文件文首均标注译自哪篇官方文档；所有叙述文字译为中文，**代码、函数签名、错误码保留英文原样**，可直接复制运行。

## 实战示例（demos/）

| 阶段 | 文件 | 主题 | 核心知识点 |
|------|------|------|-----------|
| 入门 | `demo01_hello_loop.c` | 第一个事件循环 | idle + timer、句柄引用计数 |
| 入门 | `demo02_utilities.c` | 工具函数 | `uv_hrtime`、内存/CPU 信息 |
| 入门 | `demo03_async_fs.c` | 异步文件读取 | 线程池、回调链、`uv_fs_req_cleanup` |
| 进阶 | `demo04_queue_watch.c` | 循环生命周期 | prepare/check 钩子、干净的 close 流程 |
| 进阶 | `demo05_tcp_echo.c` | TCP echo 服务器 | 每连接一句柄模式、缓冲区所有权 |
| 进阶 | `demo06_udp.c` | UDP 组播 | 数据报语义、membership |
| 进阶 | `demo07_dns.c` | 异步 DNS | `uv_getaddrinfo`、addrinfo 遍历 |
| 实战 | `demo08_pipe.c` | Unix domain socket | stream 抽象的威力 |
| 实战 | `demo09_spawn.c` | 子进程捕获输出 | stdio 重定向、管道接管 |
| 实战 | `demo10_threads.c` | 跨线程唤醒 | **libuv 并发铁律**、`uv_async_send` |
| 实战 | `demo11_signal.c` | 优雅退出 | `uv_signal_start`、SIGINT 处理 |
| 实战 | `demo12_fsevent.c` | 目录监控 | inotify 封装、UV_RENAME/UV_CHANGE |
| 高阶 | `demo13_queue_work.c` | 显式线程池 | `uv_queue_work`、`UV_THREADPOOL_SIZE` |

## 快速开始

### 准备 libuv

```bash
# 方式一：包管理器（推荐，Debian/Ubuntu）
sudo apt install libuv1-dev

# 方式二：源码编译到用户目录（无需 sudo，本仓库实战采用的方式）
git clone --depth 1 https://github.com/libuv/libuv.git
cd libuv && mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$HOME/.local
make -j$(nproc) && make install
```

### 编译运行任意示例

```bash
cd demos

# 系统安装了 libuv1-dev 时
gcc demo01_hello_loop.c -o demo01 -luv

# 源码安装到 ~/.local 时
gcc demo01_hello_loop.c -o demo01 \
    -I$HOME/.local/include -L$HOME/.local/lib \
    -luv -Wl,-rpath,$HOME/.local/lib

./demo01
```

部分示例是双端程序（TCP/UDP/管道），单独的运行说明写在每个 `.c` 文件头部注释里，配套的 `runXX.sh` 脚本可一键起服务端 + 跑客户端 + 收集双方日志。

### 重新生成 PDF

```bash
pip3 install weasyprint markdown   # 需要 Noto Sans CJK 字体
python3 build_pdf.py               # 输出 libuv-中文使用手册.pdf
```

## 阅读建议

- **零基础**：按 01 → 07 顺序读教程篇，每读完一章就跑对应的 demo。
- **有经验**：直接看「实战导学」，按需跳读，参考篇当词典查。
- **重点章节**：demo10（多线程 + async）演示的是 libuv 最容易踩坑的并发模型——事件循环只属于创建它的线程，跨线程通信只有 `uv_async_send` 一条安全通道。

## 翻译原则

- **信达雅**：叙述文字按中文技术写作习惯重写，不逐词硬译；原文的警告、注意事项、版本标注一个不漏。
- **术语统一**：event loop→事件循环，handle→句柄，request→请求，callback→回调，socket→套接字，thread pool→线程池；poll、ref/unref 等保留英文。
- **代码不译**：所有 C 代码、函数签名、参数名、错误码保留英文原样。
- **忠实原文**：官方文档中的 TODO 段落、已知错误示例（如 filesystem 章自带 `ERROR::` 演示）均按原样保留并加译注。

## 许可与致谢

- **本仓库许可：[CC BY 4.0](https://creativecommons.org/licenses/by/4.0/)** —— 可自由转载、翻译、改编（含商用），须署名并注明原文出处。
- 上游许可链：libuv 官方文档（`docs/`）本身采用 **CC Attribution 4.0 International**（见 libuv 仓库 `LICENSE-docs`），本翻译沿用相同许可；源自 `docs/code/` 的示例代码遵循 libuv 项目代码本身的许可。
- 排版：WeasyPrint + Noto Sans CJK，`build_pdf.py` 可一键复现 PDF。
- 作者：**forwaard** <forwaard@163.com> · <https://github.com/forwaard/libuv-book>
