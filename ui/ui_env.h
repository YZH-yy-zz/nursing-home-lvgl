/**
 * @file ui_env.h
 * 环境监测界面：温度/湿度/光照表盘 + WiFi 状态
 */
#ifndef UI_ENV_H
#define UI_ENV_H

#include "lvgl/lvgl.h"

lv_obj_t * ui_env_create(void);
lv_obj_t * ui_env_screen(void);

#endif /* UI_ENV_H */
