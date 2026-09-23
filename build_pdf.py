#!/usr/bin/env python3
"""组装书稿并渲染平板友好的 PDF（WeasyPrint）。

用法: python3 build_pdf.py
输入: libuv-book/*.md（按文件名排序组装）
输出: libuv-book/libuv-中文使用手册.pdf
"""
import glob
import html
import os
import re
import sys

import markdown
from weasyprint import HTML

BOOK_DIR = os.path.dirname(os.path.abspath(__file__))
OUT_PDF = os.path.join(BOOK_DIR, "libuv-中文使用手册.pdf")
OUT_HTML = os.path.join(BOOK_DIR, "book.html")

CHAPTER_TITLES = {
    "00-前言.md": "前言",
    "00a-实战导学.md": "实战导学",
    "01-事件循环与基础概念-附录-新手入门.md": "第 1 章 · libuv 基础",
    "02-事件循环详解.md": "第 2 章 · 高级事件循环",
    "03-实用工具库.md": "第 3 章 · 实用工具库",
    "04-文件系统与线程池.md": "第 4 章 · 文件系统",
    "05-网络编程.md": "第 5 章 · 网络编程",
    "06-进程管理.md": "第 6 章 · 进程",
    "07-线程与同步原语.md": "第 7 章 · 线程与同步原语",
    "08-API-事件循环.md": "第 8 章 · API：事件循环",
    "09-API-句柄与请求.md": "第 9 章 · API：句柄与请求",
    "10-API-定时器与异步唤醒.md": "第 10 章 · API：定时器与异步唤醒",
    "11-API-文件系统.md": "第 11 章 · API：文件系统",
    "12-API-文件监控.md": "第 12 章 · API：文件监控",
    "13-API-DNS与线程池及指标.md": "第 13 章 · API：DNS、线程池与指标",
    "14-API-TCP与UDP.md": "第 14 章 · API：TCP 与 UDP",
    "15-API-流与管道及轮询.md": "第 15 章 · API：流、管道、轮询与 TTY",
    "16-API-信号进程与线程.md": "第 16 章 · API：信号、进程与线程",
    "17-API-杂项错误与设计.md": "第 17 章 · API：杂项、错误码与设计概览",
}

PATTERNS = ["00-前言.md", "00a-实战导学.md"] + sorted(
    f for f in CHAPTER_TITLES if f[0].isdigit() and f not in ("00-前言.md", "00a-实战导学.md")
)

md_exts = ["extra", "codehilite", "toc", "nl2br"]
md_ext_cfg = {
    "codehilite": {"guess_lang": False, "noclasses": False},
    "toc": {"toc_depth": "2-3"},
}

def demote_headers(text: str, levels: int) -> str:
    """把 # 降 levels 级（最高到 6）。"""
    def repl(m):
        hashes = m.group(1)
        return "#" * min(6, len(hashes) + levels) + " " + m.group(2)
    return re.sub(r"^(#{1,5}) (.*)$", repl, text, flags=re.M)

def build_body() -> str:
    parts = []
    for fname in PATTERNS:
        path = os.path.join(BOOK_DIR, fname)
        raw = open(path, encoding="utf-8").read()
        chapter_no = "front" if fname.startswith("00") else "chap"
        lvl = 1 if chapter_no == "front" else 1
        content = markdown.markdown(demote_headers(raw, lvl), extensions=md_exts, extension_configs=md_ext_cfg)
        title = CHAPTER_TITLES.get(fname, fname)
        parts.append(f'<section class="chapter {chapter_no}"><h1 class="chapter-title">{html.escape(title)}</h1>{content}</section>')
    return "\n".join(parts)

CSS = """
@page {
  size: 7.55in 10in;               /* 平板 3:4 阅读比例 */
  margin: 0.7in 0.6in 0.8in 0.6in;
  @bottom-left {
    content: "forwaard <forwaard@163.com>";
    font-family: "Noto Sans CJK SC"; font-size: 7.5pt; color: #999;
  }
  @bottom-center {
    content: counter(page);
    font-family: "Noto Sans CJK SC"; font-size: 8pt; color: #888;
  }
  @bottom-right {
    content: "github.com/forwaard/libuv-book";
    font-family: "Noto Sans CJK SC"; font-size: 7.5pt; color: #999;
  }
  @top-center { content: string(chaptitle, first); font-family: "Noto Sans CJK SC"; font-size: 8pt; color: #999; }
}
@page :first { @bottom-left { content: none; } @bottom-right { content: none; } @top-center { content: none; } @bottom-center { content: none; } }
@page cover { margin: 0; @bottom-left { content: none; } @bottom-right { content: none; } @top-center { content: none; } @bottom-center { content: none; } }
@page landscape { size: 10in 7.55in; }

html { font-size: 11.5pt; }
body {
  font-family: "Noto Sans CJK SC", "Noto Serif CJK SC", sans-serif;
  color: #1a1a1a; line-height: 1.75;
  text-align: justify;
}
.cover { page: cover; width: 7.55in; height: 10in;
  background: linear-gradient(150deg, #14352c 0%, #1c5a48 55%, #2d8a6a 100%);
  color: #fff; display: block; padding: 1.2in 0.9in; box-sizing: border-box;
  string-set: chaptitle ""; }
.cover h1 { font-size: 34pt; margin: 0.6in 0 0.1in 0; font-weight: 900; letter-spacing: 2pt; }
.cover .sub { font-size: 14pt; color: #cfe8dd; margin-bottom: 0.35in; }
.cover .ver { font-size: 10.5pt; color: #9fc9b8; line-height: 1.9; }
.cover .footer { position: absolute; bottom: 0.7in; left: 0.9in; font-size: 9pt; color: #9fc9b8; }
.cover .meta { margin-top: 0.35in; font-size: 10.5pt; color: #ffd54a; line-height: 1.7; }
.cover .uv { font-family: monospace; font-size: 80pt; color: #ffd54a; font-weight: bold; margin-top: 0.4in; }

section.chapter { page-break-before: always; }
h1.chapter-title {
  font-size: 20pt; font-weight: 900; color: #14352c;
  border-bottom: 3px solid #2d8a6a; padding-bottom: 0.15in;
  margin: 0 0 0.3in 0; string-set: chaptitle content();
}
h2 { font-size: 14pt; color: #1c5a48; margin: 0.22in 0 0.08in; page-break-after: avoid;
     border-left: 4pt solid #2d8a6a; padding-left: 8pt; }
h3 { font-size: 12pt; color: #1c5a48; margin: 0.18in 0 0.06in; page-break-after: avoid; }
h4 { font-size: 11pt; color: #333; margin: 0.15in 0 0.05in; page-break-after: avoid; }
p { margin: 0.06in 0; }
a { color: #1c5a48; text-decoration: none; }
strong { color: #000; }

/* 代码块：浅底深字，等宽中文兼容 */
code { font-family: "DejaVu Sans Mono", "Noto Sans Mono CJK SC", monospace;
  font-size: 8.8pt; background: #f0f4f2; padding: 1pt 3pt; border-radius: 2pt; }
pre { background: #f7faf8; border: 1pt solid #d8e6df; border-left: 3pt solid #2d8a6a;
  border-radius: 4pt; padding: 7pt 9pt; line-height: 1.45;
  white-space: pre-wrap; word-wrap: break-word;
  font-size: 8.8pt; page-break-inside: avoid; }
pre code { background: none; padding: 0; font-size: inherit; }
.codehilite { background: #f7faf8; border-radius: 4pt; }
.codehilite pre { border: 1pt solid #d8e6df; }

/* 高亮配色（pygments class 输出） */
.codehilite .k, .codehilite .kd, .codehilite .kt { color: #00607c; font-weight: 600; }
.codehilite .nf, .codehilite .nc { color: #7a3e9d; }
.codehilite .s, .codehilite .s1, .codehilite .s2 { color: #a34d00; }
.codehilite .c, .codehilite .c1, .codehilite .cm { color: #7a7a7a; font-style: italic; }
.codehilite .mi, .codehilite .mf { color: #b03030; }
.codehilite .n  { color: #24292e; }

/* 引用块（原文注意/警告） */
blockquote { background: #fff8e8; border-left: 3pt solid #e0a800; border-radius: 3pt;
  margin: 0.08in 0; padding: 6pt 10pt; font-size: 10.3pt; page-break-inside: avoid; }
blockquote p { margin: 3pt 0; }

table { border-collapse: collapse; width: 100%; margin: 0.1in 0; font-size: 9.5pt; page-break-inside: avoid; }
th { background: #e4f0ea; color: #14352c; font-weight: 700; }
th, td { border: 1pt solid #c8dcd2; padding: 4pt 7pt; text-align: left; }
tr:nth-child(even) td { background: #f5faf7; }

ul, ol { padding-left: 1.4em; margin: 0.05in 0; }
li { margin: 2pt 0; }

hr { border: none; border-top: 1pt solid #ccc; margin: 0.15in 0; }

/* 目录 */
.toc-page { page-break-after: always; }
.toc-page h1 { font-size: 18pt; color: #14352c; border-bottom: 3px solid #2d8a6a; padding-bottom: 0.1in; }
.toc-part { font-weight: 800; color: #1c5a48; font-size: 12.5pt; margin: 0.14in 0 0.05in; }
.toc-item { font-size: 11pt; margin: 4pt 0; padding-left: 12pt; color: #333; }
.toc-item .no { display: inline-block; width: 0.55in; color: #888; }
"""

def build_toc() -> str:
    rows = ['<section class="toc-page"><h1>目录</h1>']
    parts = [
        ("导读", ["00-前言.md", "00a-实战导学.md"]),
        ("第一部分 · 教程篇（官方 Guide 全译）", [f for f in PATTERNS if f.startswith(("01", "02", "03", "04", "05", "06", "07"))]),
        ("第二部分 · 参考篇（官方 API 全译）", [f for f in PATTERNS if re.match(r"^(08|09|1\d)", f)]),
    ]
    for title, files in parts:
        rows.append(f'<div class="toc-part">{html.escape(title)}</div>')
        for f in files:
            t = CHAPTER_TITLES.get(f, f)
            no, _, rest = t.partition("·")
            no = no.strip()
            rest = rest.strip()
            if no.startswith("第"):
                disp = f'<span class="no">{html.escape(no.replace("第 ", "").replace(" 章", ""))}</span>{html.escape(rest)}'
            else:
                disp = f'<span class="no"></span>{html.escape(t)}'
            rows.append(f'<div class="toc-item">{disp}</div>')
    rows.append("</section>")
    return "\n".join(rows)

COVER = """
<section class="cover">
  <div class="uv">libuv</div>
  <h1>libuv 中文使用手册</h1>
  <div class="sub">官方文档全译 · 实战演练 13 讲</div>
  <div class="ver">
    基于 libuv 1.52.2 官方文档（commit b05973e）<br/>
    教程篇 + API 参考篇 + 实战导学<br/>
    全部示例经 Linux + GCC 编译运行验证
  </div>
  <div class="meta">forwaard &lt;forwaard@163.com&gt;<br/>github.com/forwaard/libuv-book</div>
  <div class="footer">平板阅读版 · 2026 年 9 月</div>
</section>
"""

def main():
    body = build_body()
    toc = build_toc()
    doc = f"""<!DOCTYPE html>
<html lang="zh-CN"><head><meta charset="utf-8"><style>{CSS}</style></head>
<body>
{COVER}
{toc}
{body}
</body></html>"""
    open(OUT_HTML, "w", encoding="utf-8").write(doc)
    print(f"HTML: {OUT_HTML} ({len(doc)//1024} KB)")
    HTML(string=doc, base_url=BOOK_DIR).write_pdf(OUT_PDF)
    size = os.path.getsize(OUT_PDF) / 1024 / 1024
    print(f"PDF : {OUT_PDF} ({size:.1f} MB)")

if __name__ == "__main__":
    main()
