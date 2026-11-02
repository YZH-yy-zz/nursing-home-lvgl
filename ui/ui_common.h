/**
 * @file ui_common.h
 * 公共 UI 组件：样式、顶栏、卡片、按钮、数字键盘、模态提示框等
 */
#ifndef UI_COMMON_H
#define UI_COMMON_H

#include "lvgl/lvgl.h"
#include "app_data.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 内置中文字体（wqy-microhei 定制字库，见 tools/make_font.py） */
LV_FONT_DECLARE(lv_font_cjk_16);
LV_FONT_DECLARE(lv_font_cjk_24);

/* 屏幕尺寸（阿尔法 4.3 寸 RGB 屏 800x480） */
#define UI_W   800
#define UI_H   480

/* 配色 */
#define UI_C_BG          lv_color_hex(0xEEF2F7)
#define UI_C_CARD        lv_color_hex(0xFFFFFF)
#define UI_C_PRIMARY     lv_color_hex(0x2E7DD1)
#define UI_C_PRIMARY_PR  lv_color_hex(0x1F62A8)
#define UI_C_TEAL        lv_color_hex(0x17A398)
#define UI_C_TEAL_PR     lv_color_hex(0x10837B)
#define UI_C_GREEN       lv_color_hex(0x27AE60)
#define UI_C_ORANGE      lv_color_hex(0xF2994A)
#define UI_C_RED         lv_color_hex(0xEB5757)
#define UI_C_TEXT        lv_color_hex(0x2C3E50)
#define UI_C_TEXT2       lv_color_hex(0x8A94A6)
#define UI_C_KEY         lv_color_hex(0xF1F4F9)
#define UI_C_KEY_PR      lv_color_hex(0xDDE5F0)
#define UI_C_DARK        lv_color_hex(0x1C2B3A)   /* 摄像头区域背景 */

/* 无效数值哨兵（显示为 "--"） */
#define UI_VAL_NONE  (-1000.0f)

void ui_style_init(void);

/* 全局时钟：顶栏显示当前时间（需在创建所有屏幕后调用一次） */
void ui_clock_init(void);

/* ---------- 基础组件 ---------- */
lv_obj_t * ui_screen(void);
lv_obj_t * ui_topbar(lv_obj_t * screen, const char * title, bool with_back,
                     void (*back_cb)(lv_event_t * e), lv_obj_t ** title_label);
lv_obj_t * ui_card(lv_obj_t * parent, lv_coord_t w, lv_coord_t h);
lv_obj_t * ui_btn(lv_obj_t * parent, const char * text, lv_coord_t w, lv_coord_t h);
lv_obj_t * ui_btn_ghost(lv_obj_t * parent, const char * text, lv_coord_t w, lv_coord_t h);
void ui_style_apply_btn(lv_obj_t * btn, bool primary);
lv_obj_t * ui_label(lv_obj_t * parent, const char * text, const lv_font_t * font, lv_color_t color);

/* 状态圆点（绿/红） */
lv_obj_t * ui_dot(lv_obj_t * parent, lv_coord_t x, lv_coord_t y, bool ok);

/* 模态提示框：点击"确定"后自动关闭并回调 ok_cb */
void ui_modal(lv_obj_t * parent, const char * text, void (*ok_cb)(lv_event_t * e));

/* 数字键盘：key = '0'..'9' '.' 'B'(退格) 'C'(清空) 'E'(确定) 'X'(取消) */
void ui_keypad(lv_obj_t * parent, lv_coord_t x, lv_coord_t y,
               void (*key_cb)(char key, void * user_data), void * user_data);

/* 格式化辅助：无效值输出 "--" */
const char * ui_fmt_float(char * buf, size_t n, float v, uint8_t decimals);

#ifdef __cplusplus
}
#endif

#endif /* UI_COMMON_H */
