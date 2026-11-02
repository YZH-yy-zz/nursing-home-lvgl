/**
 * @file ui_admin.h
 * 管理员端：登录 -> 病人列表 -> 检测记录查看 -> 填写检测值
 */
#ifndef UI_ADMIN_H
#define UI_ADMIN_H

#include "lvgl/lvgl.h"

lv_obj_t * ui_admin_create(void);
lv_obj_t * ui_admin_login_screen(void);
lv_obj_t * ui_admin_list_screen(void);

#endif /* UI_ADMIN_H */
