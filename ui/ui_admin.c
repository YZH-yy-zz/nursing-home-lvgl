/**
 * @file ui_admin.c
 * 管理员端：登录 -> 病人列表 -> 检测记录查看 -> 填写检测值
 *
 * 登录密码：123456（见 ADMIN_PASSWORD 宏）
 * 检测记录当前保存在内存中（app_data），将来替换为 SQLite 时
 * 本文件无需改动。
 */
#include "ui_admin.h"
#include "ui_common.h"
#include "ui_home.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ADMIN_PASSWORD "123456"

/* ---------- 屏幕 ---------- */
static lv_obj_t * s_login = NULL;
static lv_obj_t * s_list = NULL;
static lv_obj_t * s_detail = NULL;
static lv_obj_t * s_input = NULL;

/* ---------- 登录 ---------- */
static lv_obj_t * s_pwd_dots[6];
static lv_obj_t * s_pwd_hint = NULL;
static char s_pwd[8];

/* ---------- 列表 ---------- */
static lv_obj_t * s_list_cont = NULL;

/* ---------- 详情 ---------- */
static lv_obj_t * s_detail_title = NULL;
static lv_obj_t * s_detail_info = NULL;
static lv_obj_t * s_detail_today = NULL;
static lv_obj_t * s_detail_hist = NULL;
static uint8_t s_cur_patient = 0;

/* ---------- 填写检测值 ---------- */
static lv_obj_t * s_type_btns[2];
static lv_obj_t * s_type_lbls[2];
static lv_obj_t * s_input_field = NULL;
static lv_obj_t * s_input_value = NULL;
static lv_obj_t * s_input_unit = NULL;
static lv_obj_t * s_input_extra = NULL;
static lv_obj_t * s_input_hint = NULL;
static char s_input_buf[8];
static uint8_t s_input_len = 0;
static uint8_t s_input_type = 0;    /* 0 = 血糖, 1 = 血压 */
static bool s_bp_second = false;    /* 血压第二步（低压） */
static uint16_t s_bp_sys = 0;

/* ---------- 辅助 ---------- */
static lv_color_t temp_color(float t)
{
    if (t >= 36.0f && t <= 37.3f) return UI_C_GREEN;
    if (t > 37.3f && t <= 38.0f) return UI_C_ORANGE;
    return UI_C_RED;
}

/* ---------- 导航 ---------- */
static void back_home(lv_event_t * e)
{
    (void)e;
    lv_scr_load(ui_home_screen());
}

static void back_list(lv_event_t * e)
{
    (void)e;
    lv_scr_load(s_list);
}

static void back_detail(lv_event_t * e)
{
    (void)e;
    lv_scr_load(s_detail);
}

/* ================= 登录 ================= */
static void update_pwd_dots(void)
{
    size_t len = strlen(s_pwd);
    for (int i = 0; i < 6; i++) {
        lv_obj_set_style_bg_color(s_pwd_dots[i],
                                  (int)len > i ? UI_C_PRIMARY : lv_color_hex(0xD8DFEA), 0);
    }
}

static void login_try(void)
{
    if (strcmp(s_pwd, ADMIN_PASSWORD) == 0) {
        memset(s_pwd, 0, sizeof(s_pwd));
        update_pwd_dots();
        lv_scr_load(s_list);
    }
    else {
        lv_obj_clear_flag(s_pwd_hint, LV_OBJ_FLAG_HIDDEN);
        memset(s_pwd, 0, sizeof(s_pwd));
        update_pwd_dots();
    }
}

static void keypad_login_cb(char key, void * user_data)
{
    (void)user_data;

    if (key >= '0' && key <= '9') {
        size_t len = strlen(s_pwd);
        if (len < 6) {
            s_pwd[len] = key;
            s_pwd[len + 1] = '\0';
        }
        update_pwd_dots();
        if (strlen(s_pwd) == 6) login_try();   /* 输满 6 位自动提交 */
    }
    else if (key == 'B') {
        size_t len = strlen(s_pwd);
        if (len > 0) s_pwd[len - 1] = '\0';
        update_pwd_dots();
    }
    else if (key == 'C') {
        memset(s_pwd, 0, sizeof(s_pwd));
        update_pwd_dots();
    }
    else if (key == 'E') {
        login_try();
    }
    else if (key == 'X') {
        memset(s_pwd, 0, sizeof(s_pwd));
        update_pwd_dots();
        back_home(NULL);
    }
}

static void login_loaded_cb(lv_event_t * e)
{
    (void)e;
    memset(s_pwd, 0, sizeof(s_pwd));
    lv_obj_add_flag(s_pwd_hint, LV_OBJ_FLAG_HIDDEN);
    update_pwd_dots();
}

static lv_obj_t * ui_login_create(void)
{
    s_login = ui_screen();
    ui_topbar(s_login, "管理员登录", true, back_home, NULL);

    /* 密码卡片 */
    lv_obj_t * card = ui_card(s_login, 400, 300);
    lv_obj_set_pos(card, 24, 80);

    lv_obj_t * title = ui_label(card, "请输入6位密码", &lv_font_cjk_24, UI_C_TEXT);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 24);

    /* 密码圆点 */
    lv_obj_t * dots = lv_obj_create(card);
    lv_obj_remove_style_all(dots);
    lv_obj_set_size(dots, 240, 24);
    lv_obj_align(dots, LV_ALIGN_TOP_MID, 0, 84);
    lv_obj_set_flex_flow(dots, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(dots, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    for (int i = 0; i < 6; i++) {
        s_pwd_dots[i] = lv_obj_create(dots);
        lv_obj_remove_style_all(s_pwd_dots[i]);
        lv_obj_set_size(s_pwd_dots[i], 22, 22);
        lv_obj_set_style_radius(s_pwd_dots[i], LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(s_pwd_dots[i], lv_color_hex(0xD8DFEA), 0);
        lv_obj_set_style_bg_opa(s_pwd_dots[i], LV_OPA_COVER, 0);
    }

    s_pwd_hint = ui_label(card, "密码错误", &lv_font_cjk_16, UI_C_RED);
    lv_obj_align(s_pwd_hint, LV_ALIGN_TOP_MID, 0, 130);
    lv_obj_add_flag(s_pwd_hint, LV_OBJ_FLAG_HIDDEN);

    ui_keypad(s_login, 460, 80, keypad_login_cb, NULL);

    lv_obj_add_event_cb(s_login, login_loaded_cb, LV_EVENT_SCREEN_LOADED, NULL);
    return s_login;
}

/* ================= 病人列表 ================= */
static void row_click_cb(lv_event_t * e)
{
    s_cur_patient = (uint8_t)(lv_uintptr_t)lv_event_get_user_data(e);
    lv_scr_load(s_detail);
}

static void list_add_row(uint8_t idx)
{
    const app_patient_t * p = app_data_patient(idx);
    const app_record_t * r = app_data_record_today(idx);

    lv_obj_t * row = ui_card(s_list_cont, 776, 64);
    lv_obj_add_flag(row, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(row, row_click_cb, LV_EVENT_SHORT_CLICKED, (void *)(lv_uintptr_t)idx);

    lv_obj_t * name = ui_label(row, p->name, &lv_font_cjk_24, UI_C_TEXT);
    lv_obj_align(name, LV_ALIGN_LEFT_MID, 24, -10);

    char buf[64];
    snprintf(buf, sizeof(buf), "%s  %d床  %d岁", p->room, p->bed, p->age);
    lv_obj_t * info = ui_label(row, buf, &lv_font_cjk_16, UI_C_TEXT2);
    lv_obj_align(info, LV_ALIGN_LEFT_MID, 24, 14);

    lv_obj_t * tval;
    if (r != NULL && r->temp_valid) {
        snprintf(buf, sizeof(buf), "%.1f ℃", r->temp);
        tval = ui_label(row, buf, &lv_font_montserrat_24, temp_color(r->temp));
    }
    else {
        tval = ui_label(row, "未检测", &lv_font_cjk_16, UI_C_TEXT2);
    }
    lv_obj_align(tval, LV_ALIGN_RIGHT_MID, -24, 0);
}

static void list_loaded_cb(lv_event_t * e)
{
    (void)e;
    lv_obj_clean(s_list_cont);
    for (uint8_t i = 0; i < app_data_patient_count(); i++) {
        list_add_row(i);
    }
}

static lv_obj_t * ui_list_create(void)
{
    s_list = ui_screen();
    ui_topbar(s_list, "病人列表", true, back_home, NULL);

    s_list_cont = lv_obj_create(s_list);
    lv_obj_remove_style_all(s_list_cont);
    lv_obj_set_size(s_list_cont, 776, 400);
    lv_obj_set_pos(s_list_cont, 12, 68);
    lv_obj_set_flex_flow(s_list_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(s_list_cont, 10, 0);
    lv_obj_set_scrollbar_mode(s_list_cont, LV_SCROLLBAR_MODE_OFF);

    lv_obj_add_event_cb(s_list, list_loaded_cb, LV_EVENT_SCREEN_LOADED, NULL);
    return s_list;
}

/* ================= 病人详情 ================= */
static void add_today_row(lv_obj_t * parent, const char * name, bool valid, const char * value, int row)
{
    lv_obj_t * lbl = ui_label(parent, name, &lv_font_cjk_16, UI_C_TEXT2);
    lv_obj_align(lbl, LV_ALIGN_TOP_LEFT, 24, row * 26 + 4);
    lv_obj_t * val = ui_label(parent, valid ? value : "未填写", &lv_font_cjk_16,
                              valid ? UI_C_TEXT : UI_C_TEXT2);
    lv_obj_align(val, LV_ALIGN_TOP_RIGHT, -24, row * 26 + 4);
}

static void goto_input_cb(lv_event_t * e)
{
    (void)e;
    lv_scr_load(s_input);
}

static void detail_loaded_cb(lv_event_t * e)
{
    (void)e;
    const app_patient_t * p = app_data_patient(s_cur_patient);
    lv_label_set_text(s_detail_title, p->name);

    char buf[96];
    snprintf(buf, sizeof(buf), "%s  %d床  %d岁", p->room, p->bed, p->age);
    lv_label_set_text(s_detail_info, buf);

    const app_record_t * r = app_data_record_today(s_cur_patient);

    /* 今日检测记录：体温 / 血糖 / 血压 */
    lv_obj_clean(s_detail_today);
    if (r != NULL && r->temp_valid) {
        snprintf(buf, sizeof(buf), "%.1f ℃", r->temp);
        add_today_row(s_detail_today, "体温", true, buf, 0);
    }
    else {
        add_today_row(s_detail_today, "体温", false, "", 0);
    }
    if (r != NULL && r->glucose_valid) {
        snprintf(buf, sizeof(buf), "%.1f mmol/L", r->glucose);
        add_today_row(s_detail_today, "血糖", true, buf, 1);
    }
    else {
        add_today_row(s_detail_today, "血糖", false, "", 1);
    }
    if (r != NULL && r->bp_valid) {
        snprintf(buf, sizeof(buf), "%u/%u mmHg", (unsigned int)r->bp_sys, (unsigned int)r->bp_dia);
        add_today_row(s_detail_today, "血压", true, buf, 2);
    }
    else {
        add_today_row(s_detail_today, "血压", false, "", 2);
    }

    /* 历史记录（近 7 天，新的在上） */
    lv_obj_clean(s_detail_hist);
    uint8_t n = app_data_history_count(s_cur_patient);
    if (n == 0) {
        lv_obj_t * lbl = ui_label(s_detail_hist, "暂无记录", &lv_font_cjk_16, UI_C_TEXT2);
        lv_obj_align(lbl, LV_ALIGN_TOP_LEFT, 24, 4);
    }
    else {
        for (uint8_t i = 0; i < n; i++) {
            const app_record_t * hr = app_data_history(s_cur_patient, i);
            char tbuf[16], gbuf[16], bbuf[24];
            if (hr->temp_valid) {
                snprintf(tbuf, sizeof(tbuf), "%.1f℃", hr->temp);
            } else {
                snprintf(tbuf, sizeof(tbuf), "--");
            }
            if (hr->glucose_valid) {
                snprintf(gbuf, sizeof(gbuf), "%.1f", hr->glucose);
            } else {
                snprintf(gbuf, sizeof(gbuf), "--");
            }
            if (hr->bp_valid) {
                snprintf(bbuf, sizeof(bbuf), "%u/%u", (unsigned int)hr->bp_sys, (unsigned int)hr->bp_dia);
            } else {
                snprintf(bbuf, sizeof(bbuf), "--");
            }

            char line[128];
            snprintf(line, sizeof(line), "%02d-%02d  体温%s  血糖%s  血压%s",
                     hr->m, hr->d, tbuf, gbuf, bbuf);
            lv_obj_t * lbl = ui_label(s_detail_hist, line, &lv_font_cjk_16, UI_C_TEXT);
            lv_obj_align(lbl, LV_ALIGN_TOP_LEFT, 24, i * 24 + 4);
        }
    }
}

static lv_obj_t * ui_detail_create(void)
{
    s_detail = ui_screen();
    ui_topbar(s_detail, "", true, back_list, &s_detail_title);

    /* 信息条 */
    lv_obj_t * chip = ui_card(s_detail, 776, 44);
    lv_obj_set_pos(chip, 12, 68);
    s_detail_info = ui_label(chip, "", &lv_font_cjk_16, UI_C_TEXT2);
    lv_obj_center(s_detail_info);

    /* 今日检测记录卡片 */
    lv_obj_t * today_card = ui_card(s_detail, 776, 128);
    lv_obj_set_pos(today_card, 12, 124);
    lv_obj_t * t1 = ui_label(today_card, "今日检测记录", &lv_font_cjk_24, UI_C_TEXT);
    lv_obj_align(t1, LV_ALIGN_TOP_LEFT, 24, 12);
    s_detail_today = lv_obj_create(today_card);
    lv_obj_remove_style_all(s_detail_today);
    lv_obj_set_size(s_detail_today, 776, 76);
    lv_obj_set_pos(s_detail_today, 0, 48);

    /* 历史记录卡片 */
    lv_obj_t * hist_card = ui_card(s_detail, 776, 148);
    lv_obj_set_pos(hist_card, 12, 264);
    lv_obj_t * t2 = ui_label(hist_card, "近7天记录", &lv_font_cjk_24, UI_C_TEXT);
    lv_obj_align(t2, LV_ALIGN_TOP_LEFT, 24, 12);
    s_detail_hist = lv_obj_create(hist_card);
    lv_obj_remove_style_all(s_detail_hist);
    lv_obj_set_size(s_detail_hist, 776, 92);
    lv_obj_set_pos(s_detail_hist, 0, 48);
    lv_obj_set_scrollbar_mode(s_detail_hist, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scroll_dir(s_detail_hist, LV_DIR_VER);

    /* 底部按钮 */
    lv_obj_t * btn_input = ui_btn(s_detail, "填写检测值", 220, 56);
    lv_obj_align(btn_input, LV_ALIGN_TOP_MID, -120, 420);
    lv_obj_add_event_cb(btn_input, goto_input_cb, LV_EVENT_SHORT_CLICKED, NULL);

    lv_obj_t * btn_back = ui_btn_ghost(s_detail, "返回列表", 220, 56);
    lv_obj_align(btn_back, LV_ALIGN_TOP_MID, 120, 420);
    lv_obj_add_event_cb(btn_back, back_list, LV_EVENT_SHORT_CLICKED, NULL);

    lv_obj_add_event_cb(s_detail, detail_loaded_cb, LV_EVENT_SCREEN_LOADED, NULL);
    return s_detail;
}

/* ================= 填写检测值 ================= */
static void input_apply_type(void)
{
    for (int i = 0; i < 2; i++) {
        bool sel = (i == s_input_type);
        ui_style_apply_btn(s_type_btns[i], sel);
        lv_obj_set_style_text_color(s_type_lbls[i], sel ? lv_color_white() : UI_C_PRIMARY, 0);
    }
}

static void input_refresh(void)
{
    const char * field;
    const char * unit;
    const char * hint;

    if (s_input_type == 0) {
        field = "当前输入: 血糖";
        unit = "mmol/L";
        hint = "正常范围 2.0 ~ 30.0 mmol/L";
    }
    else {
        field = s_bp_second ? "当前输入: 低压" : "当前输入: 高压";
        unit = "mmHg";
        hint = s_bp_second ? "正常范围 40 ~ 160 mmHg" : "正常范围 60 ~ 260 mmHg";
    }

    lv_label_set_text(s_input_field, field);
    lv_label_set_text(s_input_unit, unit);
    lv_label_set_text(s_input_hint, hint);
    lv_obj_set_style_text_color(s_input_hint, UI_C_TEXT2, 0);

    lv_label_set_text(s_input_value, s_input_len > 0 ? s_input_buf : "--");

    if (s_input_type == 1 && s_bp_second) {
        char buf[48];
        snprintf(buf, sizeof(buf), "已填高压: %u mmHg", (unsigned int)s_bp_sys);
        lv_label_set_text(s_input_extra, buf);
        lv_obj_clear_flag(s_input_extra, LV_OBJ_FLAG_HIDDEN);
    }
    else {
        lv_obj_add_flag(s_input_extra, LV_OBJ_FLAG_HIDDEN);
    }
}

static void modal_saved_cb(lv_event_t * e)
{
    (void)e;
    back_detail(NULL);
}

static void input_commit(void)
{
    if (s_input_len == 0) {
        lv_label_set_text(s_input_hint, "请输入数值");
        lv_obj_set_style_text_color(s_input_hint, UI_C_RED, 0);
        return;
    }

    float v = (float)atof(s_input_buf);
    const char * err = "数值超出范围";

    if (s_input_type == 0) {
        /* 血糖 */
        if (v >= 2.0f && v <= 30.0f) {
            app_data_record_today_set_glucose(s_cur_patient, v);
            ui_modal(lv_scr_act(), "保存成功", modal_saved_cb);
            return;
        }
    }
    else {
        /* 血压：第一步高压，第二步低压 */
        if (!s_bp_second) {
            if (v >= 60.0f && v <= 260.0f) {
                s_bp_sys = (uint16_t)v;
                s_bp_second = true;
                s_input_len = 0;
                s_input_buf[0] = '\0';
                input_refresh();
                return;
            }
        }
        else {
            if (v >= 40.0f && v <= 160.0f) {
                if (v >= (float)s_bp_sys) {
                    err = "低压应小于高压";
                }
                else {
                    app_data_record_today_set_bp(s_cur_patient, s_bp_sys, (uint16_t)v);
                    ui_modal(lv_scr_act(), "保存成功", modal_saved_cb);
                    return;
                }
            }
        }
    }

    lv_label_set_text(s_input_hint, err);
    lv_obj_set_style_text_color(s_input_hint, UI_C_RED, 0);
}

static void keypad_input_cb(char key, void * user_data)
{
    (void)user_data;

    if (key >= '0' && key <= '9') {
        if (s_input_len < 5) {
            s_input_buf[s_input_len++] = key;
            s_input_buf[s_input_len] = '\0';
        }
        input_refresh();
    }
    else if (key == '.') {
        /* 血压为整数，小数点只用于血糖 */
        if (s_input_type == 0 && s_input_len < 5 && strchr(s_input_buf, '.') == NULL) {
            s_input_buf[s_input_len++] = '.';
            s_input_buf[s_input_len] = '\0';
        }
        input_refresh();
    }
    else if (key == 'B') {
        if (s_input_len > 0) {
            s_input_len--;
            s_input_buf[s_input_len] = '\0';
        }
        input_refresh();
    }
    else if (key == 'C') {
        s_input_len = 0;
        s_input_buf[0] = '\0';
        input_refresh();
    }
    else if (key == 'E') {
        input_commit();
    }
    else if (key == 'X') {
        back_detail(NULL);
    }
}

static void type_click_cb(lv_event_t * e)
{
    s_input_type = (uint8_t)(lv_uintptr_t)lv_event_get_user_data(e);
    s_bp_second = false;
    s_input_len = 0;
    s_input_buf[0] = '\0';
    input_apply_type();
    input_refresh();
}

static void input_loaded_cb(lv_event_t * e)
{
    (void)e;
    s_input_type = 0;
    s_bp_second = false;
    s_bp_sys = 0;
    s_input_len = 0;
    s_input_buf[0] = '\0';
    input_apply_type();
    input_refresh();
}

static void input_save_cb(lv_event_t * e)
{
    (void)e;
    input_commit();
}

static lv_obj_t * ui_input_create(void)
{
    s_input = ui_screen();
    ui_topbar(s_input, "填写检测值", true, back_detail, NULL);

    /* 类型选择（血糖 / 血压） */
    static const char * type_names[2] = {"血糖", "血压"};
    for (int i = 0; i < 2; i++) {
        s_type_btns[i] = lv_btn_create(s_input);
        lv_obj_remove_style_all(s_type_btns[i]);
        lv_obj_set_size(s_type_btns[i], 220, 52);
        lv_obj_set_pos(s_type_btns[i], 24 + i * 234, 70);
        s_type_lbls[i] = lv_label_create(s_type_btns[i]);
        lv_label_set_text(s_type_lbls[i], type_names[i]);
        lv_obj_set_style_text_font(s_type_lbls[i], &lv_font_cjk_24, 0);
        lv_obj_center(s_type_lbls[i]);
        lv_obj_add_event_cb(s_type_btns[i], type_click_cb, LV_EVENT_SHORT_CLICKED,
                            (void *)(lv_uintptr_t)i);
    }

    /* 数值显示卡片 */
    lv_obj_t * vcard = ui_card(s_input, 400, 290);
    lv_obj_set_pos(vcard, 380, 140);

    s_input_field = ui_label(vcard, "", &lv_font_cjk_16, UI_C_TEXT2);
    lv_obj_align(s_input_field, LV_ALIGN_TOP_MID, 0, 16);

    s_input_value = ui_label(vcard, "--", &lv_font_montserrat_40, UI_C_TEXT);
    lv_obj_align(s_input_value, LV_ALIGN_TOP_MID, 0, 48);

    s_input_unit = ui_label(vcard, "", &lv_font_cjk_16, UI_C_TEXT2);
    lv_obj_align(s_input_unit, LV_ALIGN_TOP_MID, 0, 108);

    s_input_extra = ui_label(vcard, "", &lv_font_cjk_16, UI_C_TEXT);
    lv_obj_align(s_input_extra, LV_ALIGN_TOP_MID, 0, 136);
    lv_obj_add_flag(s_input_extra, LV_OBJ_FLAG_HIDDEN);

    s_input_hint = ui_label(vcard, "", &lv_font_cjk_16, UI_C_TEXT2);
    lv_obj_align(s_input_hint, LV_ALIGN_TOP_MID, 0, 164);

    lv_obj_t * btn_save = ui_btn(vcard, "保存", 150, 52);
    lv_obj_align(btn_save, LV_ALIGN_BOTTOM_LEFT, 30, -16);
    lv_obj_add_event_cb(btn_save, input_save_cb, LV_EVENT_SHORT_CLICKED, NULL);

    lv_obj_t * btn_cancel = ui_btn_ghost(vcard, "取消", 150, 52);
    lv_obj_align(btn_cancel, LV_ALIGN_BOTTOM_RIGHT, -30, -16);
    lv_obj_add_event_cb(btn_cancel, back_detail, LV_EVENT_SHORT_CLICKED, NULL);

    ui_keypad(s_input, 20, 140, keypad_input_cb, NULL);

    lv_obj_add_event_cb(s_input, input_loaded_cb, LV_EVENT_SCREEN_LOADED, NULL);
    return s_input;
}

/* ================= 入口 ================= */
lv_obj_t * ui_admin_create(void)
{
    ui_login_create();
    ui_list_create();
    ui_detail_create();
    return ui_input_create();
}

lv_obj_t * ui_admin_login_screen(void)
{
    return s_login;
}

lv_obj_t * ui_admin_list_screen(void)
{
    return s_list;
}
