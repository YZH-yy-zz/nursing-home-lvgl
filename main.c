/**
 * @file main.c
 * 养老院监护系统 - 程序入口
 * 目标板：正点原子阿尔法 i.MX6ULL EMMC + 4.3 寸 RGB 屏（800x480，触摸）
 */
#include "lvgl/lvgl.h"
#include "lv_drivers/display/fbdev.h"
#include "lv_drivers/indev/evdev.h"
#include "ui/ui_app.h"
#include "custom.h"
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

#define DISP_BUF_SIZE (800 * 480)

int main(void)
{
    /*LVGL init*/
    lv_init();

    /*Linux frame buffer device init*/
    fbdev_init();

    /*读取 framebuffer 的真实分辨率（不能写死，否则画面会超出屏幕）*/
    uint32_t fb_w = 800, fb_h = 480, fb_dpi = 130;
    fbdev_get_sizes(&fb_w, &fb_h, &fb_dpi);
    printf("fbdev: %u x %u, dpi %u\n", (unsigned int)fb_w, (unsigned int)fb_h, (unsigned int)fb_dpi);

    /*A full-screen buffer for LVGL to draw the screen's content*/
    static lv_color_t buf[DISP_BUF_SIZE];

    /*Initialize a descriptor for the buffer*/
    static lv_disp_draw_buf_t disp_buf;
    lv_disp_draw_buf_init(&disp_buf, buf, NULL, DISP_BUF_SIZE);

    /*Initialize and register a display driver*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.draw_buf   = &disp_buf;
    disp_drv.flush_cb   = fbdev_flush;
    disp_drv.hor_res    = (lv_coord_t)fb_w;
    disp_drv.ver_res    = (lv_coord_t)fb_h;
    lv_disp_drv_register(&disp_drv);

    /*Touch screen (evdev)*/
    evdev_init();
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv); /*Basic initialization*/
    indev_drv.type = LV_INDEV_TYPE_POINTER;

    /*This function will be called periodically (by the library) to get the touch position and state*/
    indev_drv.read_cb = evdev_read;
    lv_indev_drv_register(&indev_drv);

    /*若调试时连接了 USB 鼠标需要光标，取消下面注释即可：
    LV_IMG_DECLARE(mouse_cursor_icon)
    lv_obj_t * cursor_obj = lv_img_create(lv_scr_act());
    lv_img_set_src(cursor_obj, &mouse_cursor_icon);
    lv_indev_set_cursor(mouse_indev, cursor_obj);
    */

    /*Create the nursing-home monitoring UI*/
    ui_app_create();

    /*Handle LVGL tasks (tickless mode)*/
    while(1) {
        lv_timer_handler();
        usleep(5000);
    }

    return 0;
}

/*Set in lv_conf.h as `LV_TICK_CUSTOM_SYS_TIME_EXPR`*/
uint32_t custom_tick_get(void)
{
    static uint64_t start_ms = 0;
    if(start_ms == 0) {
        struct timeval tv_start;
        gettimeofday(&tv_start, NULL);
        start_ms = (tv_start.tv_sec * 1000000 + tv_start.tv_usec) / 1000;
    }

    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);
    uint64_t now_ms;
    now_ms = (tv_now.tv_sec * 1000000 + tv_now.tv_usec) / 1000;

    uint32_t time_ms = now_ms - start_ms;
    return time_ms;
}
