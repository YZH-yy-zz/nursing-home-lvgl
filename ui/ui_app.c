/**
 * @file ui_app.c
 * 应用入口：初始化数据层、样式、时钟，创建所有界面并显示首页
 */
#include "ui_app.h"
#include "ui_common.h"
#include "ui_home.h"
#include "ui_env.h"
#include "ui_patient.h"
#include "ui_admin.h"
#include "app_data.h"

void ui_app_create(void)
{
    app_data_init();
    ui_style_init();
    ui_clock_init();

    lv_obj_t * home = ui_home_create();
    ui_env_create();
    ui_patient_create();
    ui_admin_create();

    lv_scr_load(home);
}
