#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Check which characters of the UI strings are covered by a LVGL font source file.

Usage: python check_font.py <path-to-lv_font_*.c>

Parses the font's cmap tables the same way lv_font_fmt_txt.c does:
  - FORMAT0_* : codepoints [range_start, range_start + range_length)
  - SPARSE_*   : unicode_list holds values RELATIVE to range_start
                 (real codepoint = range_start + list value)
Then reports any character of the UI strings (shared with make_font.py)
that is not covered.
"""
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from make_font import STRINGS, EXTRA  # noqa: E402


def main() -> int:
    if len(sys.argv) < 2:
        print("Usage: python check_font.py <lv_font_*.c>")
        return 1

    with open(sys.argv[1], encoding="utf-8", errors="replace") as f:
        src = f.read()

    # 1) Collect all unicode_list arrays: name -> [values].
    lists = {}
    for m in re.finditer(r"(unicode_list_\d+)\[\]\s*=\s*\{(.*?)\}\s*;", src, re.S):
        values = [int(x, 16) for x in re.findall(r"0x([0-9A-Fa-f]+)", m.group(2))]
        lists[m.group(1)] = values

    # 2) Parse each cmap table entry.
    cmap_re = re.compile(
        r"\.range_start\s*=\s*(\d+)\s*,\s*\.range_length\s*=\s*(\d+)"
        r".*?\.unicode_list\s*=\s*(\w+)\s*,"
        r".*?\.type\s*=\s*(\w+)",
        re.S,
    )
    covered = set()
    for m in cmap_re.finditer(src):
        rstart = int(m.group(1))
        rlen = int(m.group(2))
        ulist = m.group(3)
        ctype = m.group(4)
        if "FORMAT0" in ctype:
            covered.update(range(rstart, rstart + rlen))
        elif "SPARSE" in ctype and ulist in lists:
            for v in lists[ulist]:
                covered.add(rstart + v)
        else:
            print(f"WARNING: unhandled cmap entry (start={rstart}, type={ctype})")

    if not covered:
        print("ERROR: no cmap data found - wrong file or format?")
        return 2

    print(f"Font covers {len(covered)} codepoints.")

    all_text = "".join(STRINGS) + EXTRA
    missing = sorted({ch for ch in all_text if not ch.isspace() and ord(ch) not in covered})

    if missing:
        print("\n=== MISSING CHARACTERS ===")
        print("Chars :", "".join(missing))
        print("Codepoints:", [f"U+{ord(c):04X}" for c in missing])
        return 3

    print("\nAll UI strings fully covered.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
