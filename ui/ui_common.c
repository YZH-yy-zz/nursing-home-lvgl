/**
 * @file ui_common.c
 * 公共 UI 组件实现
 */
#include "ui_common.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

/* ---------- 样式 ---------- */
static lv_style_t st_screen;
static lv_style_t st_card;
static lv_style_t st_topbar;
static lv_style_t st_btn, st_btn_pr;
static lv_style_t st_btn_g, st_btn_g_pr;
static lv_style_t st_key, st_key_pr;
static lv_style_t st_key_enter, st_key_enter_pr;

/* 顶栏时钟标签（时钟定时器更新所有注册的标签） */
#define UI_MAX_CLOCKS 8
static lv_obj_t * s_clocks[UI_MAX_CLOCKS];
static uint8_t s_clock_cnt;

/* ---------- 键盘上下文（每个键盘实例一份） ---------- */
typedef struct {
    void (*key_cb)(char key, void * user_data);
    void * user_data;
} ui_keypad_ctx_t;

#define UI_KEYPAD_MAX 4
static ui_keypad_ctx_t s_keypad_ctxs[UI_KEYPAD_MAX];
static uint8_t s_keypad_cnt;

/* ---------- 模态框上下文 ---------- */
typedef struct {
    lv_obj_t * overlay;
    void (*ok_cb)(lv_event_t * e);
} ui_modal_ctx_t;
static ui_modal_ctx_t s_modal;

void ui_style_init(void)
{
    /* 屏幕背景 */
    lv_style_init(&st_screen);
    lv_style_set_bg_color(&st_screen, UI_C_BG);
    lv_style_set_bg_opa(&st_screen, LV_OPA_COVER);
    lv_style_set_border_width(&st_screen, 0);
    lv_style_set_pad_all(&st_screen, 0);

    /* 白色卡片 */
    lv_style_init(&st_card);
    lv_style_set_bg_color(&st_card, UI_C_CARD);
    lv_style_set_bg_opa(&st_card, LV_OPA_COVER);
    lv_style_set_radius(&st_card, 12);
    lv_style_set_border_width(&st_card, 0);
    lv_style_set_pad_all(&st_card, 0);
    lv_style_set_shadow_width(&st_card, 12);
    lv_style_set_shadow_opa(&st_card, 25);
    lv_style_set_shadow_ofs_y(&st_card, 3);

    /* 顶栏 */
    lv_style_init(&st_topbar);
    lv_style_set_bg_color(&st_topbar, UI_C_PRIMARY);
    lv_style_set_bg_opa(&st_topbar, LV_OPA_COVER);
    lv_style_set_radius(&st_topbar, 0);
    lv_style_set_border_width(&st_topbar, 0);
    lv_style_set_pad_all(&st_topbar, 0);

    /* 主按钮 */
    lv_style_init(&st_btn);
    lv_style_set_bg_color(&st_btn, UI_C_PRIMARY);
    lv_style_set_bg_opa(&st_btn, LV_OPA_COVER);
    lv_style_set_radius(&st_btn, 10);
    lv_style_set_border_width(&st_btn, 0);
    lv_style_set_pad_all(&st_btn, 0);
    lv_style_set_shadow_width(&st_btn, 6);
    lv_style_set_shadow_opa(&st_btn, 40);
    lv_style_set_shadow_ofs_y(&st_btn, 2);

    lv_style_init(&st_btn_pr);
    lv_style_set_bg_color(&st_btn_pr, UI_C_PRIMARY_PR);
    lv_style_set_bg_opa(&st_btn_pr, LV_OPA_COVER);

    /* 次按钮（幽灵风格） */
    lv_style_init(&st_btn_g);
    lv_style_set_bg_color(&st_btn_g, UI_C_CARD);
    lv_style_set_bg_opa(&st_btn_g, LV_OPA_COVER);
    lv_style_set_radius(&st_btn_g, 10);
    lv_style_set_border_width(&st_btn_g, 2);
    lv_style_set_border_color(&st_btn_g, UI_C_PRIMARY);
    lv_style_set_pad_all(&st_btn_g, 0);

    lv_style_init(&st_btn_g_pr);
    lv_style_set_bg_color(&st_btn_g_pr, lv_color_hex(0xE3ECF7));
    lv_style_set_bg_opa(&st_btn_g_pr, LV_OPA_COVER);

    /* 键盘按键 */
    lv_style_init(&st_key);
    lv_style_set_bg_color(&st_key, UI_C_KEY);
    lv_style_set_bg_opa(&st_key, LV_OPA_COVER);
    lv_style_set_radius(&st_key, 8);
    lv_style_set_border_width(&st_key, 0);

    lv_style_init(&st_key_pr);
    lv_style_set_bg_color(&st_key_pr, UI_C_KEY_PR);
    lv_style_set_bg_opa(&st_key_pr, LV_OPA_COVER);

    lv_style_init(&st_key_enter);
    lv_style_set_bg_color(&st_key_enter, UI_C_TEAL);
    lv_style_set_bg_opa(&st_key_enter, LV_OPA_COVER);
    lv_style_set_radius(&st_key_enter, 8);
    lv_style_set_border_width(&st_key_enter, 0);

    lv_style_init(&st_key_enter_pr);
    lv_style_set_bg_color(&st_key_enter_pr, UI_C_TEAL_PR);
    lv_style_set_bg_opa(&st_key_enter_pr, LV_OPA_COVER);
}

/* ---------- 时钟 ---------- */
static void clock_timer_cb(lv_timer_t * t)
{
    (void)t;
    time_t now = time(NULL);
    struct tm tmv;
    localtime_r(&now, &tmv);

    for (uint8_t i = 0; i < s_clock_cnt; i++) {
        if (s_clocks[i] != NULL) {
            lv_label_set_text_fmt(s_clocks[i], "%02d:%02d:%02d",
                                  tmv.tm_hour, tmv.tm_min, tmv.tm_sec);
        }
    }
}

void ui_clock_init(void)
{
    lv_timer_create(clock_timer_cb, 1000, NULL);
}

/* ---------- 屏幕 ---------- */
lv_obj_t * ui_screen(void)
{
    lv_obj_t * scr = lv_obj_create(NULL);
    lv_obj_remove_style_all(scr);
    lv_obj_add_style(scr, &st_screen, 0);
    return scr;
}

/* ---------- 顶栏 ---------- */
lv_obj_t * ui_topbar(lv_obj_t * screen, const char * title, bool with_back,
                     void (*back_cb)(lv_event_t * e), lv_obj_t ** title_label)
{
    lv_obj_t * bar = lv_obj_create(screen);
    lv_obj_remove_style_all(bar);
    lv_obj_add_style(bar, &st_topbar, 0);
    lv_obj_set_size(bar, UI_W, 56);
    lv_obj_set_pos(bar, 0, 0);
    lv_obj_clear_flag(bar, LV_OBJ_FLAG_SCROLLABLE);

    if (with_back) {
        lv_obj_t * bk = lv_btn_create(bar);
        lv_obj_remove_style_all(bk);
        lv_obj_set_style_bg_opa(bk, LV_OPA_TRANSP, 0);
        lv_obj_set_style_shadow_width(bk, 0, 0);
        lv_obj_set_style_bg_color(bk, UI_C_PRIMARY_PR, LV_STATE_PRESSED);
        lv_obj_set_style_bg_opa(bk, LV_OPA_COVER, LV_STATE_PRESSED);
        lv_obj_set_size(bk, 84, 56);
        lv_obj_set_pos(bk, 0, 0);
        if (back_cb != NULL) {
            lv_obj_add_event_cb(bk, back_cb, LV_EVENT_SHORT_CLICKED, NULL);
        }

        lv_obj_t * arrow = lv_label_create(bk);
        lv_label_set_text(arrow, LV_SYMBOL_LEFT);
        lv_obj_set_style_text_font(arrow, &lv_font_montserrat_24, 0);
        lv_obj_set_style_text_color(arrow, lv_color_white(), 0);
        lv_obj_center(arrow);
    }

    lv_obj_t * t = lv_label_create(bar);
    lv_label_set_text(t, title);
    lv_obj_set_style_text_font(t, &lv_font_cjk_24, 0);
    lv_obj_set_style_text_color(t, lv_color_white(), 0);
    lv_obj_align(t, LV_ALIGN_LEFT_MID, with_back ? 92 : 24, 0);
    if (title_label != NULL) *title_label = t;

    /* 时钟（注册到全局时钟定时器） */
    lv_obj_t * clock = lv_label_create(bar);
    lv_label_set_text(clock, "--:--:--");
    lv_obj_set_style_text_font(clock, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(clock, lv_color_white(), 0);
    lv_obj_align(clock, LV_ALIGN_RIGHT_MID, -24, 0);
    if (s_clock_cnt < UI_MAX_CLOCKS) {
        s_clocks[s_clock_cnt++] = clock;
    }

    return bar;
}

/* ---------- 卡片 ---------- */
lv_obj_t * ui_card(lv_obj_t * parent, lv_coord_t w, lv_coord_t h)
{
    lv_obj_t * c = lv_obj_create(parent);
    lv_obj_remove_style_all(c);
    lv_obj_add_style(c, &st_card, 0);
    lv_obj_set_size(c, w, h);
    lv_obj_clear_flag(c, LV_OBJ_FLAG_SCROLLABLE);
    return c;
}

/* ---------- 按钮 ---------- */
static lv_obj_t * btn_create(lv_obj_t * parent, const char * text,
                             lv_coord_t w, lv_coord_t h, bool primary)
{
    lv_obj_t * b = lv_btn_create(parent);
    lv_obj_remove_style_all(b);
    ui_style_apply_btn(b, primary);
    lv_obj_set_size(b, w, h);

    lv_obj_t * lbl = lv_label_create(b);
    lv_label_set_text(lbl, text);
    lv_obj_set_style_text_font(lbl, &lv_font_cjk_24, 0);
    lv_obj_set_style_text_color(lbl, primary ? lv_color_white() : UI_C_PRIMARY, 0);
    lv_obj_center(lbl);
    return b;
}

lv_obj_t * ui_btn(lv_obj_t * parent, const char * text, lv_coord_t w, lv_coord_t h)
{
    return btn_create(parent, text, w, h, true);
}

lv_obj_t * ui_btn_ghost(lv_obj_t * parent, const char * text, lv_coord_t w, lv_coord_t h)
{
    return btn_create(parent, text, w, h, false);
}

/* 切换按钮主/次风格（用于分段选择按钮） */
void ui_style_apply_btn(lv_obj_t * btn, bool primary)
{
    lv_obj_remove_style_all(btn);
    if (primary) {
        lv_obj_add_style(btn, &st_btn, 0);
        lv_obj_add_style(btn, &st_btn_pr, LV_STATE_PRESSED);
    }
    else {
        lv_obj_add_style(btn, &st_btn_g, 0);
        lv_obj_add_style(btn, &st_btn_g_pr, LV_STATE_PRESSED);
    }
}

/* ---------- 标签 ---------- */
lv_obj_t * ui_label(lv_obj_t * parent, const char * text, const lv_font_t * font, lv_color_t color)
{
    lv_obj_t * lbl = lv_label_create(parent);
    lv_label_set_text(lbl, text);
    lv_obj_set_style_text_font(lbl, font, 0);
    lv_obj_set_style_text_color(lbl, color, 0);
    return lbl;
}

/* ---------- 状态圆点 ---------- */
lv_obj_t * ui_dot(lv_obj_t * parent, lv_coord_t x, lv_coord_t y, bool ok)
{
    lv_obj_t * d = lv_obj_create(parent);
    lv_obj_remove_style_all(d);
    lv_obj_set_size(d, 14, 14);
    lv_obj_set_pos(d, x, y);
    lv_obj_set_style_radius(d, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(d, ok ? UI_C_GREEN : UI_C_RED, 0);
    lv_obj_set_style_bg_opa(d, LV_OPA_COVER, 0);
    return d;
}

/* ---------- 模态提示框 ---------- */
static void modal_ok_handler(lv_event_t * e)
{
    (void)e;
    lv_obj_del(s_modal.overlay);
    if (s_modal.ok_cb != NULL) {
        s_modal.ok_cb(e);
    }
}

void ui_modal(lv_obj_t * parent, const char * text, void (*ok_cb)(lv_event_t * e))
{
    /* 全屏半透明遮罩，挡住下层触摸 */
    s_modal.overlay = lv_obj_create(parent);
    lv_obj_remove_style_all(s_modal.overlay);
    lv_obj_set_style_bg_color(s_modal.overlay, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(s_modal.overlay, LV_OPA_40, 0);
    lv_obj_set_size(s_modal.overlay, UI_W, UI_H);
    lv_obj_set_pos(s_modal.overlay, 0, 0);
    lv_obj_clear_flag(s_modal.overlay, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(s_modal.overlay, LV_OBJ_FLAG_CLICKABLE);
    s_modal.ok_cb = ok_cb;

    lv_obj_t * card = ui_card(s_modal.overlay, 420, 220);
    lv_obj_center(card);

    lv_obj_t * lbl = ui_label(card, text, &lv_font_cjk_24, UI_C_TEXT);
    lv_obj_set_width(lbl, 360);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(lbl, LV_ALIGN_TOP_MID, 0, 44);

    lv_obj_t * btn = ui_btn(card, "确定", 160, 56);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -28);
    lv_obj_add_event_cb(btn, modal_ok_handler, LV_EVENT_SHORT_CLICKED, NULL);
}

/* ---------- 数字键盘 ---------- */
static void keypad_key_handler(lv_event_t * e)
{
    /* 上下文存放在键盘容器对象的 user_data 上（登录键盘/输入键盘各自独立） */
    lv_obj_t * kp = lv_obj_get_parent(lv_event_get_target(e));
    ui_keypad_ctx_t * ctx = (ui_keypad_ctx_t *)lv_obj_get_user_data(kp);
    char * key = (char *)lv_event_get_user_data(e);
    if (ctx != NULL && key != NULL && ctx->key_cb != NULL) {
        ctx->key_cb(*key, ctx->user_data);
    }
}

/* 键盘布局：4 行 x 4 列（0 键占 2 列） */
typedef struct {
    const char * label;      /* 显示文本 */
    bool cjk;                /* 中文标签用 CJK 字体 */
    char key;                /* 回调键值 */
    uint8_t span;            /* 列跨度 */
} keypad_key_t;

static const keypad_key_t keypad_keys[] = {
    /* 1 2 3 ⌫ */
    {"1", false, '1', 1}, {"2", false, '2', 1}, {"3", false, '3', 1}, {LV_SYMBOL_BACKSPACE, false, 'B', 1},
    /* 4 5 6 清除 */
    {"4", false, '4', 1}, {"5", false, '5', 1}, {"6", false, '6', 1}, {"清除", true, 'C', 1},
    /* 7 8 9 . */
    {"7", false, '7', 1}, {"8", false, '8', 1}, {"9", false, '9', 1}, {".", false, '.', 1},
    /* 取消 0(跨2列) 确定 */
    {"取消", true, 'X', 1}, {"0", false, '0', 2}, {"确定", true, 'E', 1},
};

void ui_keypad(lv_obj_t * parent, lv_coord_t x, lv_coord_t y,
               void (*key_cb)(char key, void * user_data), void * user_data)
{
    /* 从静态池取一个上下文（本应用最多 2 个键盘） */
    ui_keypad_ctx_t * ctx =
        &s_keypad_ctxs[s_keypad_cnt < UI_KEYPAD_MAX ? s_keypad_cnt++ : UI_KEYPAD_MAX - 1];
    ctx->key_cb = key_cb;
    ctx->user_data = user_data;

    lv_obj_t * kp = lv_obj_create(parent);
    lv_obj_remove_style_all(kp);
    lv_obj_set_size(kp, 340, 300);
    lv_obj_set_pos(kp, x, y);
    lv_obj_clear_flag(kp, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_user_data(kp, ctx);

    static lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static lv_coord_t row_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    lv_obj_set_layout(kp, LV_LAYOUT_GRID);
    lv_obj_set_grid_dsc_array(kp, col_dsc, row_dsc);
    lv_obj_set_style_pad_column(kp, 6, 0);
    lv_obj_set_style_pad_row(kp, 6, 0);

    uint8_t col = 0, row = 0;
    for (uint32_t i = 0; i < sizeof(keypad_keys) / sizeof(keypad_keys[0]); i++) {
        const keypad_key_t * k = &keypad_keys[i];
        if (col + k->span > 4) {   /* 换行 */
            col = 0;
            row++;
        }

        lv_obj_t * b = lv_btn_create(kp);
        lv_obj_remove_style_all(b);
        if (k->key == 'E') {
            lv_obj_add_style(b, &st_key_enter, 0);
            lv_obj_add_style(b, &st_key_enter_pr, LV_STATE_PRESSED);
        }
        else {
            lv_obj_add_style(b, &st_key, 0);
            lv_obj_add_style(b, &st_key_pr, LV_STATE_PRESSED);
        }
        lv_obj_set_grid_cell(b, LV_GRID_ALIGN_STRETCH, col, k->span,
                             LV_GRID_ALIGN_STRETCH, row, 1);
        lv_obj_add_event_cb(b, keypad_key_handler, LV_EVENT_SHORT_CLICKED,
                            (void *)&k->key);

        lv_obj_t * lbl = lv_label_create(b);
        lv_label_set_text(lbl, k->label);
        lv_obj_set_style_text_font(lbl, k->cjk ? &lv_font_cjk_16 : &lv_font_montserrat_24, 0);
        lv_obj_set_style_text_color(lbl, k->key == 'E' ? lv_color_white() : UI_C_TEXT, 0);
        lv_obj_center(lbl);

        col += k->span;
    }
}

/* ---------- 格式化辅助 ---------- */
const char * ui_fmt_float(char * buf, size_t n, float v, uint8_t decimals)
{
    if (v <= UI_VAL_NONE / 2.0f) {
        snprintf(buf, n, "--");
    }
    else if (decimals == 0) {
        snprintf(buf, n, "%.0f", v);
    }
    else {
        snprintf(buf, n, "%.*f", (int)decimals, v);
    }
    return buf;
}
