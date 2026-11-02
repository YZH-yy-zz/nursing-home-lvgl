#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
校验源码字符串字面量中用到的所有非 ASCII 字符是否都在字体字符集里。

Usage: python check_strings.py
（字符集取自 tools/chars.txt，与 make_font.py 生成的字体一致）
"""
import io
import re
import sys

sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding="utf-8", errors="replace")

FILES = [
    "app_data.c",
    "app_data.h",
    "main.c",
    "ui/ui_app.c",
    "ui/ui_common.c",
    "ui/ui_home.c",
    "ui/ui_env.c",
    "ui/ui_patient.c",
    "ui/ui_admin.c",
]

chars = set(open("tools/chars.txt", encoding="utf-8").read())

used = set()
for f in FILES:
    src = open(f, encoding="utf-8").read()
    # 去掉注释，避免注释里的汉字干扰
    s = re.sub(r"/\*.*?\*/", "", src, flags=re.S)
    s = re.sub(r"//.*", "", s)
    # 提取所有字符串字面量中的非 ASCII 字符
    for m in re.finditer(r'"((?:[^"\\]|\\.)*)"', s):
        for ch in m.group(1):
            if ord(ch) > 127:
                used.add(ch)

missing = sorted(used - chars)
print("字符串字面量中的非 ASCII 字符总数:", len(used))
if missing:
    print("字体未覆盖（必须加入 make_font.py）:", "".join(missing))
    sys.exit(1)
print("字体未覆盖: (无) — 全部字符串可正常显示")
