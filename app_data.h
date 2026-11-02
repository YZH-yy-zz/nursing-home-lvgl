/**
 * @file app_data.h
 * 养老院监护系统 - 数据层接口
 *
 * 说明：当前为演示用的内存数据层：
 *   - 环境数据由 app_data_env_sim() 模拟（将来由 STM32 通过 WiFi 上报，
 *     接收任务调用 app_data_env_set() 写入即可）
 *   - 检测记录保存在内存中（将来可替换为 SQLite，只需重写本文件的实现）
 */
#ifndef APP_DATA_H
#define APP_DATA_H

#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#define APP_PATIENT_NUM    6     /* 病人数量 */
#define APP_NAME_LEN       16    /* 姓名最长字节数 */
#define APP_ROOM_LEN       16    /* 房间号最长字节数 */
#define APP_HISTORY_DAYS   7     /* 保留最近 7 天的记录 */

/* 环境数据（温度/湿度/光照，由 STM32 通过 WiFi 上报） */
typedef struct {
    float    temp;              /* 温度 ℃ */
    float    humi;              /* 湿度 %  */
    uint32_t light;             /* 光照 lx */
    bool     wifi_connected;    /* 与 STM32 的 WiFi 连接状态 */
} app_env_t;

/* 病人基本信息 */
typedef struct {
    char    name[APP_NAME_LEN];
    char    room[APP_ROOM_LEN];
    uint8_t bed;
    uint8_t age;
} app_patient_t;

/* 单日检测记录（将来对应 SQLite 中的一行） */
typedef struct {
    uint16_t y;                 /* 日期：年 */
    uint8_t  m;                 /* 日期：月 */
    uint8_t  d;                 /* 日期：日 */
    bool     temp_valid;        /* 体温（病人每日检测自动记录） */
    float    temp;
    bool     glucose_valid;     /* 血糖（管理员填写） */
    float    glucose;
    bool     bp_valid;          /* 血压（管理员填写） */
    uint16_t bp_sys;            /* 高压 */
    uint16_t bp_dia;            /* 低压 */
} app_record_t;

void app_data_init(void);

/* ---------- 环境数据 ---------- */
void app_data_env_sim(void);
void app_data_env_set(float temp, float humi, uint32_t light, bool wifi_connected);
const app_env_t * app_data_env(void);

/* ---------- 时间（板子 RTC 时间） ---------- */
void app_data_now(struct tm * t);

/* ---------- 病人 ---------- */
uint8_t app_data_patient_count(void);
const app_patient_t * app_data_patient(uint8_t idx);

/* 人脸识别：返回识别到的病人编号（预留：对接真实人脸识别模块） */
uint8_t app_data_face_recognize(void);

/* ---------- 检测记录 ---------- */
app_record_t * app_data_record_today(uint8_t patient_idx);
bool app_data_record_today_add_temp(uint8_t patient_idx, float temp);
void app_data_record_today_set_glucose(uint8_t patient_idx, float v);
void app_data_record_today_set_bp(uint8_t patient_idx, uint16_t sys, uint16_t dia);

/* 历史记录：按日期从新到旧，idx 从 0 开始 */
uint8_t app_data_history_count(uint8_t patient_idx);
const app_record_t * app_data_history(uint8_t patient_idx, uint8_t idx);

#ifdef __cplusplus
}
#endif

#endif /* APP_DATA_H */
