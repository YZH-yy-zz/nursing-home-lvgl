#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Generate the character-set file for lv_font_conv from all UI strings.

Usage: python make_font.py > tools/chars.txt
Then : lv_font_conv --font tools/fonts/wqy-microhei.ttc --size 16 --bpp 4 \
                    --format lvgl --symbols tools/chars.txt -r 0x20-0x7F \
                    --output ui/lv_font_cjk_16.c --no-compress

Add any new UI text to STRINGS below and re-run both commands.
"""
import sys
import io

sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding="utf-8", errors="replace")

STRINGS = [
    # ---- home ----
    "养老院监护系统",
    "环境监测", "温度湿度光照",
    "病人检测", "人脸识别体温",
    "管理员", "记录查询填写",
    "室内环境",
    # ---- environment ----
    "温度", "湿度", "光照",
    "已连接", "未连接", "数据来源", "模拟数据",
    # ---- common ----
    "返回", "返回首页", "确定", "取消", "保存", "已保存",
    "保存成功", "提示", "成功",
    "年月日时分秒",
    # ---- patient flow ----
    "人脸识别", "请正对摄像头", "正在识别", "识别成功",
    "体温检测", "开始测量", "测量中", "检测成功", "已自动记录",
    "今日已检测", "再次检测", "返回首页",
    "请将体温计对准额头后开始测量", "测量时请保持静止",
    "房间", "床位", "岁", "栋", "室", "床",
    # ---- admin ----
    "管理员登录", "请输入位密码", "密码错误",
    "病人列表", "未检测",
    "今日检测记录", "近天记录", "体温", "血糖", "血压",
    "高压", "低压", "未填写", "暂无记录",
    "填写检测值", "数值超出范围", "请输入数值",
    "记录成功", "今日体温已记录",
    "姓名", "返回列表", "清除", "当前输入", "正常范围",
    "低压应小于高压", "已填高压",
    # ---- units / symbols ----
    "℃", "°C",
    # ---- demo patients ----
    "张三", "李四", "王五", "赵六", "钱七", "孙八",
]

# Extra chars kept available for future use (dates, warnings, etc.)
EXTRA = "年月日 上午下午 已取消 删除 添加 修改 退出 关闭 帮助 关于"

ALL = "".join(STRINGS) + EXTRA

chars = []
for ch in ALL:
    if ch not in chars and not ch.isspace():
        chars.append(ch)

print("".join(chars), end="")
