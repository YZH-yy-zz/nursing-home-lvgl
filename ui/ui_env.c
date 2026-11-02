/**
 * @file ui_env.c
 * 环境监测界面：温度/湿度/光照表盘 + WiFi 状态
 * 数据来自 app_data（当前为模拟，将来为 STM32 通过 WiFi 上报）
 */
#include "ui_env.h"
#include "ui_common.h"
#include "ui_home.h"
#include <stdio.h>

#define GAUGE_NUM 3

static lv_obj_t * s_env = NULL;
static lv_timer_t * s_timer = NULL;
static lv_obj_t * s_wifi_dot = NULL;
static lv_obj_t * s_wifi_label = NULL;

typedef struct {
    lv_obj_t * arc;
    lv_obj_t * value;
} gauge_t;

static gauge_t s_gauges[GAUGE_NUM];

typedef struct {
    const char * title;
    int16_t min;
    int16_t max;
    const char * unit;
} gauge_def_t;

static const gauge_def_t defs[GAUGE_NUM] = {
    {"温度", -10,  50,   "℃"},
    {"湿度",   0, 100,   "%"},
    {"光照",   0, 1000,  "lx"},
};

static void back_home(lv_event_t * e)
{
    (void)e;
    lv_scr_load(ui_home_screen());
}

/* ---------- 定时刷新 ---------- */
static void env_timer_cb(lv_timer_t * t)
{
    (void)t;
    if (lv_scr_act() != s_env) return;

    app_data_env_sim();
    const app_env_t * e = app_data_env();

    float vals[GAUGE_NUM] = {e->temp, e->humi, (float)e->light};
    for (int i = 0; i < GAUGE_NUM; i++) {
        int16_t v = (int16_t)LV_CLAMP(defs[i].min, vals[i], defs[i].max);
        lv_arc_set_value(s_gauges[i].arc, v);

        char buf[16];
        if (i == 2) {
            snprintf(buf, sizeof(buf), "%u", (unsigned int)e->light);
        }
        else {
            snprintf(buf, sizeof(buf), "%.1f", vals[i]);
        }
        lv_label_set_text(s_gauges[i].value, buf);

        /* 阈值着色（lv_color_t 是结构体，不能用 == 比较，用标志位） */
        lv_color_t c = UI_C_TEXT;
        bool normal = true;
        if (i == 0 && vals[i] > 30.0f) { c = UI_C_ORANGE; normal = false; }
        if (i == 0 && vals[i] > 38.0f) { c = UI_C_RED;    normal = false; }
        if (i == 1 && vals[i] > 75.0f) { c = UI_C_ORANGE; normal = false; }
        if (i == 2 && vals[i] > 800.0f) { c = UI_C_ORANGE; normal = false; }

        lv_obj_set_style_text_color(s_gauges[i].value, c, 0);
        lv_obj_set_style_arc_color(s_gauges[i].arc,
                                   normal ? UI_C_PRIMARY : c,
                                   LV_PART_INDICATOR | LV_STATE_DEFAULT);
    }

    /* WiFi 状态 */
    lv_obj_set_style_bg_color(s_wifi_dot, e->wifi_connected ? UI_C_GREEN : UI_C_RED, 0);
    lv_label_set_text(s_wifi_label, e->wifi_connected ? "WiFi 已连接" : "WiFi 未连接");
}

/* ---------- 创建界面 ---------- */
lv_obj_t * ui_env_create(void)
{
    s_env = ui_screen();
    ui_topbar(s_env, "环境监测", true, back_home, NULL);

    /* 三个表盘卡片 */
    lv_coord_t x = 16;
    for (int i = 0; i < GAUGE_NUM; i++) {
        lv_obj_t * card = ui_card(s_env, 240, 260);
        lv_obj_set_pos(card, x, 76);
        x += 240 + 24;

        lv_obj_t * title = ui_label(card, defs[i].title, &lv_font_cjk_16, UI_C_TEXT2);
        lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 14);

        lv_obj_t * arc = lv_arc_create(card);
        lv_obj_set_size(arc, 140, 140);
        lv_obj_align(arc, LV_ALIGN_CENTER, 0, 12);
        lv_arc_set_range(arc, defs[i].min, defs[i].max);
        lv_arc_set_value(arc, 0);
        lv_obj_remove_style(arc, NULL, LV_PART_KNOB);
        lv_obj_clear_flag(arc, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_arc_width(arc, 12, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_arc_color(arc, lv_color_hex(0xE4E9F1), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_arc_width(arc, 12, LV_PART_INDICATOR | LV_STATE_DEFAULT);
        lv_obj_set_style_arc_color(arc, UI_C_PRIMARY, LV_PART_INDICATOR | LV_STATE_DEFAULT);

        lv_obj_t * value = ui_label(card, "--", &lv_font_montserrat_30, UI_C_TEXT);
        lv_obj_align_to(value, arc, LV_ALIGN_CENTER, 0, 0);

        lv_obj_t * unit = ui_label(card, defs[i].unit, &lv_font_cjk_16, UI_C_TEXT2);
        lv_obj_align(unit, LV_ALIGN_BOTTOM_MID, 0, -16);

        s_gauges[i].arc = arc;
        s_gauges[i].value = value;
    }

    /* WiFi 状态行 */
    lv_obj_t * row = ui_card(s_env, 776, 48);
    lv_obj_set_pos(row, 12, 356);
    s_wifi_dot = ui_dot(row, 20, 17, true);
    s_wifi_label = ui_label(row, "WiFi 已连接", &lv_font_cjk_16, UI_C_TEXT);
    lv_obj_align(s_wifi_label, LV_ALIGN_LEFT_MID, 48, 0);
    lv_obj_t * src = ui_label(row, "数据来源 STM32 模拟", &lv_font_cjk_16, UI_C_TEXT2);
    lv_obj_align(src, LV_ALIGN_RIGHT_MID, -20, 0);

    s_timer = lv_timer_create(env_timer_cb, 2000, NULL);
    lv_timer_ready(s_timer);   /* 立即刷新一次 */

    return s_env;
}

lv_obj_t * ui_env_screen(void)
{
    return s_env;
}
