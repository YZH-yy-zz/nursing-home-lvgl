/**
 * @file ui_patient.h
 * 病人端：人脸识别 -> 体温检测 -> 自动记录
 */
#ifndef UI_PATIENT_H
#define UI_PATIENT_H

#include "lvgl/lvgl.h"

lv_obj_t * ui_patient_create(void);
lv_obj_t * ui_face_screen(void);
lv_obj_t * ui_temp_screen(void);

#endif /* UI_PATIENT_H */
