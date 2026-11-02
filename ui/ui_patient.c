/**
 * @file ui_patient.c
 * 病人端：人脸识别 -> 体温检测 -> 自动记录
 *
 * 人脸识别为模拟流程（app_data_face_recognize 轮流返回病人），
 * 将来接入真实人脸识别模块时只需替换数据层实现。
 */
#include "ui_patient.h"
#include "ui_common.h"
#include "ui_home.h"
#include <stdio.h>
#include <stdlib.h>

/* ---------- 屏幕 ---------- */
static lv_obj_t * s_face = NULL;
static lv_obj_t * s_temp = NULL;

/* ---------- 人脸识别 ---------- */
static lv_obj_t * s_face_status = NULL;
static lv_timer_t * s_face_t1 = NULL;
static lv_timer_t * s_face_t2 = NULL;
static bool s_face_done = false;

/* ---------- 体温检测 ---------- */
static lv_obj_t * s_temp_patient = NULL;
static lv_obj_t * s_temp_value = NULL;
static lv_obj_t * s_temp_status = NULL;
static lv_obj_t * s_btn_start = NULL;
static lv_obj_t * s_btn_again = NULL;
static lv_obj_t * s_btn_home = NULL;
static lv_timer_t * s_meas_timer = NULL;

static uint8_t s_patient_id = 0;
static float s_current_temp = UI_VAL_NONE;
static float s_meas_target = 36.5f;
static uint8_t s_meas_step = 0;

typedef enum {
    TEMP_IDLE,        /* 待测量 */
    TEMP_MEASURING,   /* 测量中 */
    TEMP_DONE,        /* 测量完成并已记录 */
    TEMP_RECORDED,    /* 今日已检测 */
} temp_state_t;

static temp_state_t s_state = TEMP_IDLE;

/* ---------- 前置声明 ---------- */
static void face_goto_temp_cb(lv_timer_t * t);
static void meas_tick_cb(lv_timer_t * t);

/* ---------- 辅助 ---------- */
static void back_home(lv_event_t * e)
{
    (void)e;
    lv_scr_load(ui_home_screen());
}

static lv_color_t temp_color(float t)
{
    if (t >= 36.0f && t <= 37.3f) return UI_C_GREEN;
    if (t > 37.3f && t <= 38.0f) return UI_C_ORANGE;
    return UI_C_RED;
}

static void show_temp_value(void)
{
    char buf[16];
    lv_label_set_text(s_temp_value, ui_fmt_float(buf, sizeof(buf), s_current_temp, 1));

    lv_color_t c;
    if (s_state == TEMP_MEASURING) {
        c = UI_C_ORANGE;
    }
    else if (s_state == TEMP_IDLE) {
        c = UI_C_TEXT;
    }
    else {
        c = temp_color(s_current_temp);
    }
    lv_obj_set_style_text_color(s_temp_value, c, 0);
}

/* ================= 人脸识别 ================= */
static void face_do_recognize(void)
{
    if (s_face_done) return;
    s_face_done = true;

    s_patient_id = app_data_face_recognize();
    const app_patient_t * p = app_data_patient(s_patient_id);

    char buf[48];
    snprintf(buf, sizeof(buf), "识别成功  %s", p->name);
    lv_label_set_text(s_face_status, buf);
    lv_obj_set_style_text_color(s_face_status, lv_color_hex(0x66E08A), 0);

    if (s_face_t1 != NULL) {
        lv_timer_del(s_face_t1);
        s_face_t1 = NULL;
    }

    s_face_t2 = lv_timer_create(face_goto_temp_cb, 1200, NULL);
    lv_timer_set_repeat_count(s_face_t2, 1);
}

static void face_goto_temp_cb(lv_timer_t * t)
{
    (void)t;
    s_face_t2 = NULL;
    lv_scr_load(s_temp);
}

static void face_recognize_cb(lv_timer_t * t)
{
    (void)t;
    s_face_t1 = NULL;
    face_do_recognize();
}

/* 点击摄像头区域：跳过等待直接识别（演示方便） */
static void face_skip_cb(lv_event_t * e)
{
    (void)e;
    face_do_recognize();
}

static void face_loaded_cb(lv_event_t * e)
{
    (void)e;
    s_face_done = false;
    lv_label_set_text(s_face_status, "正在识别");
    lv_obj_set_style_text_color(s_face_status, lv_color_white(), 0);
    s_face_t1 = lv_timer_create(face_recognize_cb, 2500, NULL);
    lv_timer_set_repeat_count(s_face_t1, 1);
}

static void face_unloaded_cb(lv_event_t * e)
{
    (void)e;
    if (s_face_t1 != NULL) { lv_timer_del(s_face_t1); s_face_t1 = NULL; }
    if (s_face_t2 != NULL) { lv_timer_del(s_face_t2); s_face_t2 = NULL; }
}

/* ================= 体温检测 ================= */
static void finish_measure(void)
{
    s_current_temp = s_meas_target + ((float)(rand() % 10) - 5.0f) * 0.02f;
    s_state = TEMP_DONE;
    /* 记录当日体温（每天只记录第一次，重复测量不覆盖） */
    app_data_record_today_add_temp(s_patient_id, s_current_temp);
    show_temp_value();

    lv_label_set_text(s_temp_status, "检测成功 已自动记录");
    lv_obj_set_style_text_color(s_temp_status, UI_C_GREEN, 0);

    lv_obj_clear_flag(s_btn_again, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(s_btn_home, LV_OBJ_FLAG_HIDDEN);
}

static void meas_tick_cb(lv_timer_t * t)
{
    s_meas_step++;
    float progress = (float)s_meas_step / 10.0f;
    if (progress > 1.0f) progress = 1.0f;
    s_current_temp = 35.5f + (s_meas_target - 35.5f) * progress;
    show_temp_value();

    if (s_meas_step >= 10) {
        lv_timer_del(t);
        s_meas_timer = NULL;
        finish_measure();
    }
}

static void start_measure(void)
{
    s_state = TEMP_MEASURING;
    s_meas_step = 0;
    s_meas_target = 36.2f + (float)(rand() % 9) * 0.1f;   /* 36.2 ~ 37.0 */
    s_current_temp = 35.5f;
    show_temp_value();

    lv_obj_add_flag(s_btn_start, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(s_btn_again, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(s_btn_home, LV_OBJ_FLAG_HIDDEN);

    lv_label_set_text(s_temp_status, "测量中");
    lv_obj_set_style_text_color(s_temp_status, UI_C_ORANGE, 0);

    if (s_meas_timer == NULL) {
        s_meas_timer = lv_timer_create(meas_tick_cb, 150, NULL);
    }
}

static void start_meas_cb(lv_event_t * e)
{
    (void)e;
    start_measure();
}

static void again_cb(lv_event_t * e)
{
    (void)e;
    start_measure();
}

static void temp_loaded_cb(lv_event_t * e)
{
    (void)e;
    const app_patient_t * p = app_data_patient(s_patient_id);

    char buf[64];
    snprintf(buf, sizeof(buf), "%s  %s  %d床", p->name, p->room, p->bed);
    lv_label_set_text(s_temp_patient, buf);

    const app_record_t * r = app_data_record_today(s_patient_id);
    if (r != NULL && r->temp_valid) {
        /* 今日已检测：只展示结果，不允许重复记录 */
        s_state = TEMP_RECORDED;
        s_current_temp = r->temp;
        show_temp_value();
        lv_label_set_text(s_temp_status, "今日已检测 体温已记录");
        lv_obj_set_style_text_color(s_temp_status, UI_C_GREEN, 0);
        lv_obj_add_flag(s_btn_start, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(s_btn_again, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(s_btn_home, LV_OBJ_FLAG_HIDDEN);
    }
    else {
        s_state = TEMP_IDLE;
        s_current_temp = UI_VAL_NONE;
        show_temp_value();
        lv_label_set_text(s_temp_status, "请将体温计对准额头后开始测量");
        lv_obj_set_style_text_color(s_temp_status, UI_C_TEXT2, 0);
        lv_obj_clear_flag(s_btn_start, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(s_btn_again, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(s_btn_home, LV_OBJ_FLAG_HIDDEN);
    }
}

static void temp_unloaded_cb(lv_event_t * e)
{
    (void)e;
    if (s_meas_timer != NULL) {
        lv_timer_del(s_meas_timer);
        s_meas_timer = NULL;
    }
}

/* ---------- 创建界面 ---------- */
static lv_obj_t * ui_face_create(void)
{
    s_face = ui_screen();
    ui_topbar(s_face, "病人检测", true, back_home, NULL);

    /* 摄像头区域卡片 */
    lv_obj_t * cam = ui_card(s_face, 480, 320);
    lv_obj_set_pos(cam, 160, 80);
    lv_obj_set_style_bg_color(cam, UI_C_DARK, 0);
    lv_obj_set_style_bg_opa(cam, LV_OPA_COVER, 0);
    lv_obj_add_flag(cam, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(cam, face_skip_cb, LV_EVENT_SHORT_CLICKED, NULL);

    lv_obj_t * icon = ui_label(cam, LV_SYMBOL_VIDEO, &lv_font_montserrat_48, lv_color_white());
    lv_obj_align(icon, LV_ALIGN_TOP_MID, 0, 60);

    lv_obj_t * hint = ui_label(cam, "请正对摄像头", &lv_font_cjk_24, lv_color_white());
    lv_obj_align(hint, LV_ALIGN_TOP_MID, 0, 160);

    s_face_status = ui_label(cam, "正在识别", &lv_font_cjk_16, lv_color_white());
    lv_obj_align(s_face_status, LV_ALIGN_BOTTOM_MID, 0, -36);

    lv_obj_add_event_cb(s_face, face_loaded_cb, LV_EVENT_SCREEN_LOADED, NULL);
    lv_obj_add_event_cb(s_face, face_unloaded_cb, LV_EVENT_SCREEN_UNLOADED, NULL);
    return s_face;
}

static lv_obj_t * ui_temp_create(void)
{
    s_temp = ui_screen();
    ui_topbar(s_temp, "体温检测", true, back_home, NULL);

    /* 病人信息条 */
    lv_obj_t * chip = ui_card(s_temp, 460, 44);
    lv_obj_set_pos(chip, 170, 72);
    s_temp_patient = ui_label(chip, "", &lv_font_cjk_16, UI_C_TEXT);
    lv_obj_center(s_temp_patient);

    /* 大温度显示（数值 + 单位水平居中） */
    lv_obj_t * disp = lv_obj_create(s_temp);
    lv_obj_remove_style_all(disp);
    lv_obj_set_size(disp, 300, 80);
    lv_obj_set_pos(disp, 250, 146);
    lv_obj_set_flex_flow(disp, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(disp, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(disp, 8, 0);

    s_temp_value = ui_label(disp, "--", &lv_font_montserrat_48, UI_C_TEXT);
    ui_label(disp, "℃", &lv_font_cjk_24, UI_C_TEXT2);

    /* 状态提示 */
    s_temp_status = ui_label(s_temp, "", &lv_font_cjk_16, UI_C_TEXT2);
    lv_obj_align(s_temp_status, LV_ALIGN_TOP_MID, 0, 250);

    /* 按钮 */
    s_btn_start = ui_btn(s_temp, "开始测量", 240, 64);
    lv_obj_align(s_btn_start, LV_ALIGN_TOP_MID, 0, 300);
    lv_obj_add_event_cb(s_btn_start, start_meas_cb, LV_EVENT_SHORT_CLICKED, NULL);

    s_btn_again = ui_btn_ghost(s_temp, "再次检测", 220, 64);
    lv_obj_align(s_btn_again, LV_ALIGN_TOP_MID, -120, 300);
    lv_obj_add_event_cb(s_btn_again, again_cb, LV_EVENT_SHORT_CLICKED, NULL);

    s_btn_home = ui_btn(s_temp, "返回首页", 220, 64);
    lv_obj_align(s_btn_home, LV_ALIGN_TOP_MID, 120, 300);
    lv_obj_add_event_cb(s_btn_home, back_home, LV_EVENT_SHORT_CLICKED, NULL);

    lv_obj_add_event_cb(s_temp, temp_loaded_cb, LV_EVENT_SCREEN_LOADED, NULL);
    lv_obj_add_event_cb(s_temp, temp_unloaded_cb, LV_EVENT_SCREEN_UNLOADED, NULL);
    return s_temp;
}

lv_obj_t * ui_patient_create(void)
{
    ui_face_create();
    return ui_temp_create();
}

lv_obj_t * ui_face_screen(void)
{
    return s_face;
}

lv_obj_t * ui_temp_screen(void)
{
    return s_temp;
}
