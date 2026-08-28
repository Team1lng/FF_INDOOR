#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Export indoor-machine UI strings (English + Persian) from layout/language.c
into a valid .xlsx using openpyxl (guaranteed Excel-openable)."""

import re
import os
import openpyxl
from openpyxl.styles import Font, Alignment

BASE = os.path.dirname(os.path.abspath(__file__))
H_FILE = os.path.join(BASE, "layout", "language.h")
C_FILE = os.path.join(BASE, "layout", "language.c")
OUT = os.path.join(os.path.expanduser("~"), "Desktop", "language.xlsx")


def parse_enum_ids(path):
    text = open(path, encoding="utf-8").read()
    m = re.search(r"\} language_type;.*?typedef\s+enum\s*\{(.*?)\}\s*layout_lang_id\s*;",
                  text, re.S)
    if not m:
        raise RuntimeError("layout_lang_id enum not found in " + path)
    body = m.group(1)
    ids = []
    for line in body.splitlines():
        line = line.split("//", 1)[0].strip()
        if not line:
            continue
        mm = re.match(r"([A-Za-z_][A-Za-z0-9_]*)\s*,", line)
        if mm and mm.group(1) != "LANG_STR_ID_TOTAL":
            ids.append(mm.group(1))
    return ids


def parse_lang_array(path):
    text = open(path, encoding="utf-8").read()
    m = re.search(r"lang_str\s*\[\s*LANG_STR_ID_TOTAL\s*\]\s*\[\s*LANG_TOTAL\s*\]\s*=\s*\{(.*?)\};",
                  text, re.S)
    if not m:
        raise RuntimeError("lang_str array not found in " + path)
    body = m.group(1)
    rows = []
    for ln in body.splitlines():
        code = ln.split("//", 1)[0]
        if "{" not in code or '"' not in code:
            continue
        parts = re.findall(r'"((?:[^"\\]|\\.)*)"', code)
        eng = parts[0] if len(parts) >= 1 else ""
        per = parts[1] if len(parts) >= 2 else ""
        rows.append((eng, per))
    return rows


def main():
    ids = parse_enum_ids(H_FILE)
    data = parse_lang_array(C_FILE)
    print("enum ids:", len(ids), " array rows:", len(data))
    n = min(len(ids), len(data))

    wb = openpyxl.Workbook()
    ws = wb.active
    ws.title = "Indoor Strings"

    headers = ["ID (enum)", "English / 中文界面", "Persian (فارسی)"]
    ws.append(headers)
    for c in range(1, 4):
        cell = ws.cell(row=1, column=c)
        cell.font = Font(bold=True)
        cell.alignment = Alignment(horizontal="center", vertical="center")

    for i in range(n):
        ws.append([ids[i], data[i][0], data[i][1]])

    # reasonable column widths
    ws.column_dimensions["A"].width = 34
    ws.column_dimensions["B"].width = 40
    ws.column_dimensions["C"].width = 40

    if os.path.exists(OUT):
        os.remove(OUT)
    wb.save(OUT)
    print("OK: wrote %s (%d string rows)" % (OUT, n))


if __name__ == "__main__":
    main()
