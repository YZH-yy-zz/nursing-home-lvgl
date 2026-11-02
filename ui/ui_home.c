/**
 * @file ui_home.c
 * 首页：三个功能入口 + 环境数据摘要
 */
#include "ui_home.h"
#include "ui_common.h"
#include "ui_env.h"
#include "ui_patient.h"
#include "ui_admin.h"
#include <stdio.h>

static lv_obj_t * s_home = NULL;
static lv_obj_t * s_summary = NULL;
static lv_obj_t * s_summary_dot = NULL;
static lv_timer_t * s_timer = NULL;

/* ---------- 导航 ---------- */
static void nav_env(lv_event_t * e)     { (void)e; lv_scr_load(ui_env_screen()); }
static void nav_patient(lv_event_t * e) { (void)e; lv_scr_load(ui_face_screen()); }
static void nav_admin(lv_event_t * e)   { (void)e; lv_scr_load(ui_admin_login_screen()); }

/* ---------- 环境摘要定时刷新 ---------- */
static void home_env_timer_cb(lv_timer_t * t)
{
    (void)t;
    if (lv_scr_act() != s_home) return;

    app_data_env_sim();
    const app_env_t * e = app_data_env();

    char buf[96];
    snprintf(buf, sizeof(buf), "室内环境  温度 %.1f℃    湿度 %.0f%%    光照 %u lx",
             e->temp, e->humi, (unsigned int)e->light);
    lv_label_set_text(s_summary, buf);
    lv_obj_set_style_bg_color(s_summary_dot, e->wifi_connected ? UI_C_GREEN : UI_C_RED, 0);
}

/* ---------- 创建首页 ---------- */
lv_obj_t * ui_home_create(void)
{
    s_home = ui_screen();
    ui_topbar(s_home, "养老院监护系统", false, NULL, NULL);

    /* 三个入口卡片 */
    typedef struct {
        const char * sym;
        const char * title;
        const char * sub;
        void (*cb)(lv_event_t * e);
    } menu_t;

    static const menu_t menus[] = {
        {LV_SYMBOL_WIFI,     "环境监测", "温度 湿度 光照",    nav_env},
        {LV_SYMBOL_VIDEO,    "病人检测", "人脸识别 体温",     nav_patient},
        {LV_SYMBOL_SETTINGS, "管理员",   "记录查询 填写",     nav_admin},
    };

    lv_coord_t x = 12;
    for (uint32_t i = 0; i < sizeof(menus) / sizeof(menus[0]); i++) {
        lv_obj_t * card = ui_card(s_home, 244, 300);
        lv_obj_set_pos(card, x, 76);
        lv_obj_add_flag(card, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(card, menus[i].cb, LV_EVENT_SHORT_CLICKED, NULL);

        lv_obj_t * icon = ui_label(card, menus[i].sym, &lv_font_montserrat_48, UI_C_PRIMARY);
        lv_obj_align(icon, LV_ALIGN_TOP_MID, 0, 44);

        lv_obj_t * title = ui_label(card, menus[i].title, &lv_font_cjk_24, UI_C_TEXT);
        lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 140);

        lv_obj_t * sub = ui_label(card, menus[i].sub, &lv_font_cjk_16, UI_C_TEXT2);
        lv_obj_align(sub, LV_ALIGN_TOP_MID, 0, 192);

        x += 244 + 22;
    }

    /* 底部环境摘要条 */
    lv_obj_t * sum_card = ui_card(s_home, 776, 76);
    lv_obj_set_pos(sum_card, 12, 392);
    s_summary_dot = ui_dot(sum_card, 24, 31, true);
    s_summary = ui_label(sum_card, "室内环境  --", &lv_font_cjk_16, UI_C_TEXT);
    lv_obj_align(s_summary, LV_ALIGN_LEFT_MID, 56, 0);

    s_timer = lv_timer_create(home_env_timer_cb, 2000, NULL);
    lv_timer_ready(s_timer);   /* 立即刷新一次 */

    return s_home;
}

lv_obj_t * ui_home_screen(void)
{
    return s_home;
}
