/**
 * @file app_data.c
 * 养老院监护系统 - 数据层实现（内存版，见 app_data.h 说明）
 */
#include "app_data.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>

/* ---------- 病人信息（演示数据） ---------- */
static const app_patient_t patients[APP_PATIENT_NUM] = {
    {"张三", "3栋101室", 1, 78},
    {"李四", "3栋101室", 2, 82},
    {"王五", "3栋102室", 1, 75},
    {"赵六", "3栋102室", 2, 80},
    {"钱七", "2栋201室", 1, 85},
    {"孙八", "2栋201室", 2, 72},
};

/* ---------- 环境数据 ---------- */
static app_env_t env = {26.3f, 52.0f, 320, true};
static float sim_phase = 0.0f;

/* ---------- 检测记录 ---------- */
static app_record_t records[APP_PATIENT_NUM][APP_HISTORY_DAYS];

/* 人脸识别轮转（演示） */
static uint8_t next_face_patient = 0;

/* 公历日期 -> 从 1970-01-01 起的天数（Howard Hinnant 算法，年份恒为非负） */
static uint32_t days_from_civil(uint16_t y, uint8_t m, uint8_t d)
{
    y -= m <= 2;
    uint32_t era = (uint32_t)y / 400;
    uint32_t yoe = (uint32_t)(y - era * 400);
    uint32_t doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    uint32_t doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + doe - 719468;
}

void app_data_init(void)
{
    memset(records, 0, sizeof(records));
    srand(20260101u);   /* 固定种子，让模拟数据可复现 */
}

/* ---------- 时间 ---------- */
void app_data_now(struct tm * t)
{
    time_t now = time(NULL);
    localtime_r(&now, t);
}

/* ---------- 环境数据 ---------- */
void app_data_env_set(float temp, float humi, uint32_t light, bool wifi_connected)
{
    env.temp = temp;
    env.humi = humi;
    env.light = light;
    env.wifi_connected = wifi_connected;
}

/* 模拟 STM32 上报的环境数据。
 * 将来接入真实系统时：由 WiFi/串口接收线程解析数据后调用
 * app_data_env_set() 写入，本函数即可删除。 */
void app_data_env_sim(void)
{
    sim_phase += 0.25f;
    float temp  = 24.5f + 2.5f * sinf(sim_phase * 0.6f) + ((float)(rand() % 10) - 5.0f) * 0.06f;
    float humi  = 50.0f + 9.0f * sinf(sim_phase * 0.9f) + ((float)(rand() % 10) - 5.0f) * 0.08f;
    uint32_t light = (uint32_t)(400.0f + 300.0f * (0.5f + 0.5f * sinf(sim_phase * 1.4f))) + rand() % 20;
    app_data_env_set(temp, humi, light, true);
}

const app_env_t * app_data_env(void)
{
    return &env;
}

/* ---------- 病人 ---------- */
uint8_t app_data_patient_count(void)
{
    return APP_PATIENT_NUM;
}

const app_patient_t * app_data_patient(uint8_t idx)
{
    if (idx >= APP_PATIENT_NUM) idx = 0;
    return &patients[idx];
}

/* 人脸识别（预留对接点）：真实系统中由人脸识别模块返回病人编号，
 * 这里轮流返回各病人用于演示。 */
uint8_t app_data_face_recognize(void)
{
    uint8_t id = next_face_patient;
    next_face_patient = (uint8_t)((next_face_patient + 1) % APP_PATIENT_NUM);
    return id;
}

/* ---------- 检测记录 ---------- */
app_record_t * app_data_record_today(uint8_t patient_idx)
{
    if (patient_idx >= APP_PATIENT_NUM) return NULL;

    struct tm t;
    app_data_now(&t);
    uint16_t y = (uint16_t)(t.tm_year + 1900);
    uint8_t m = (uint8_t)(t.tm_mon + 1);
    uint8_t d = (uint8_t)t.tm_mday;

    /* 1. 已有今天的记录 */
    for (int i = 0; i < APP_HISTORY_DAYS; i++) {
        app_record_t * r = &records[patient_idx][i];
        if (r->y == y && r->m == m && r->d == d) return r;
    }

    /* 2. 找空槽位 */
    for (int i = 0; i < APP_HISTORY_DAYS; i++) {
        if (records[patient_idx][i].y == 0) {
            app_record_t * r = &records[patient_idx][i];
            r->y = y; r->m = m; r->d = d;
            return r;
        }
    }

    /* 3. 槽位已满：覆盖最旧的一天 */
    int oldest = 0;
    uint32_t oldest_days = days_from_civil(records[patient_idx][0].y,
                                           records[patient_idx][0].m,
                                           records[patient_idx][0].d);
    for (int i = 1; i < APP_HISTORY_DAYS; i++) {
        app_record_t * r = &records[patient_idx][i];
        uint32_t dd = days_from_civil(r->y, r->m, r->d);
        if (dd < oldest_days) {
            oldest_days = dd;
            oldest = i;
        }
    }
    app_record_t * r = &records[patient_idx][oldest];
    r->y = y; r->m = m; r->d = d;
    r->temp_valid = r->glucose_valid = r->bp_valid = false;
    return r;
}

/* 记录当日体温：每天只记录一次，返回 false 表示今天已记录过 */
bool app_data_record_today_add_temp(uint8_t patient_idx, float temp)
{
    app_record_t * r = app_data_record_today(patient_idx);
    if (r == NULL || r->temp_valid) return false;
    r->temp = temp;
    r->temp_valid = true;
    return true;
}

void app_data_record_today_set_glucose(uint8_t patient_idx, float v)
{
    app_record_t * r = app_data_record_today(patient_idx);
    if (r == NULL) return;
    r->glucose = v;
    r->glucose_valid = true;
}

void app_data_record_today_set_bp(uint8_t patient_idx, uint16_t sys, uint16_t dia)
{
    app_record_t * r = app_data_record_today(patient_idx);
    if (r == NULL) return;
    r->bp_sys = sys;
    r->bp_dia = dia;
    r->bp_valid = true;
}

uint8_t app_data_history_count(uint8_t patient_idx)
{
    if (patient_idx >= APP_PATIENT_NUM) return 0;
    uint8_t n = 0;
    for (int i = 0; i < APP_HISTORY_DAYS; i++) {
        if (records[patient_idx][i].y != 0) n++;
    }
    return n;
}

/* 返回按日期从新到旧排序的第 idx 条记录 */
const app_record_t * app_data_history(uint8_t patient_idx, uint8_t idx)
{
    if (patient_idx >= APP_PATIENT_NUM) return NULL;

    /* 收集有效记录，按日期降序插入排序 */
    static app_record_t sorted[APP_HISTORY_DAYS];
    uint8_t n = 0;
    for (int i = 0; i < APP_HISTORY_DAYS; i++) {
        app_record_t * r = &records[patient_idx][i];
        if (r->y == 0) continue;

        uint8_t j = n;
        uint32_t days = days_from_civil(r->y, r->m, r->d);
        while (j > 0) {
            app_record_t * prev = &sorted[j - 1];
            uint32_t prev_days = days_from_civil(prev->y, prev->m, prev->d);
            if (prev_days >= days) break;
            sorted[j] = sorted[j - 1];
            j--;
        }
        sorted[j] = *r;
        n++;
    }

    if (idx >= n) return NULL;
    return &sorted[idx];
}
