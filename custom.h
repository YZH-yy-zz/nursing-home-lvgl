/**
 * @file custom.h
 * Declares the platform-specific tick source implemented in main.c.
 *
 * Included by lvgl/src/hal/lv_hal_tick.c through `LV_TICK_CUSTOM_INCLUDE`
 * (see lv_conf.h). Keeping the prototype here (instead of relying on an
 * implicit declaration) makes the build work with modern GCC, which treats
 * implicit function declarations as an error.
 */
#ifndef CUSTOM_H
#define CUSTOM_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Returns the elapsed time in milliseconds since start-up (see main.c). */
uint32_t custom_tick_get(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CUSTOM_H */
