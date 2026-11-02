#
# Makefile
#
# Cross-compilation for i.MX6ULL (ARM Cortex-A7) etc.:
#   make CROSS_COMPILE=arm-linux-gnueabihf-
# (leave CROSS_COMPILE empty to build natively on the host with gcc)
CROSS_COMPILE ?=
CC = $(CROSS_COMPILE)gcc

LVGL_DIR_NAME ?= lvgl
LVGL_DIR ?= ${shell pwd}
CFLAGS ?= -O3 -g0 -std=gnu99 -I$(LVGL_DIR)/ -I$(LVGL_DIR)/ui -Wall -Wshadow -Wundef -Wmissing-prototypes -Wall -Wextra -Wno-unused-function -Wno-error=strict-prototypes -Wpointer-arith -fno-strict-aliasing -Wno-error=cpp -Wuninitialized -Wmaybe-uninitialized -Wno-unused-parameter -Wno-missing-field-initializers -Wtype-limits -Wsizeof-pointer-memaccess -Wno-format-nonliteral -Wno-cast-qual -Wunreachable-code -Wno-switch-default -Wreturn-type -Wmultichar -Wformat-security -Wno-ignored-qualifiers -Wno-error=pedantic -Wno-sign-compare -Wno-error=missing-prototypes -Wdouble-promotion -Wclobbered -Wdeprecated -Wempty-body -Wtype-limits -Wstack-usage=2048 -Wno-unused-value -Wno-unused-parameter -Wno-missing-field-initializers -Wuninitialized -Wmaybe-uninitialized -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wtype-limits -Wsizeof-pointer-memaccess -Wno-format-nonliteral -Wpointer-arith -Wno-cast-qual -Wmissing-prototypes -Wunreachable-code -Wno-switch-default -Wreturn-type -Wmultichar -Wformat-security -Wno-ignored-qualifiers -Wno-sign-compare -Wno-double-promotion
LDFLAGS ?= -lm
BIN = demo


#Collect the files to compile
MAINSRC = ./main.c

include $(LVGL_DIR)/lvgl/lvgl.mk
include $(LVGL_DIR)/lv_drivers/lv_drivers.mk

#Application sources (nursing-home monitoring system)
CSRCS += $(LVGL_DIR)/app_data.c
CSRCS += $(LVGL_DIR)/ui/ui_app.c
CSRCS += $(LVGL_DIR)/ui/ui_common.c
CSRCS += $(LVGL_DIR)/ui/ui_home.c
CSRCS += $(LVGL_DIR)/ui/ui_env.c
CSRCS += $(LVGL_DIR)/ui/ui_patient.c
CSRCS += $(LVGL_DIR)/ui/ui_admin.c
CSRCS += $(LVGL_DIR)/ui/lv_font_cjk_16.c
CSRCS += $(LVGL_DIR)/ui/lv_font_cjk_24.c

#Mouse cursor icon (only needed when a mouse cursor is enabled in main.c)
#CSRCS += $(LVGL_DIR)/mouse_cursor_icon.c

OBJEXT ?= .o

AOBJS = $(ASRCS:.S=$(OBJEXT))
COBJS = $(CSRCS:.c=$(OBJEXT))

MAINOBJ = $(MAINSRC:.c=$(OBJEXT))

SRCS = $(ASRCS) $(CSRCS) $(MAINSRC)
OBJS = $(AOBJS) $(COBJS)

## MAINOBJ -> OBJFILES

all: default

%.o: %.c
	@$(CC)  $(CFLAGS) -c $< -o $@
	@echo "CC $<"
    
default: $(AOBJS) $(COBJS) $(MAINOBJ)
	$(CC) -o $(BIN) $(MAINOBJ) $(AOBJS) $(COBJS) $(LDFLAGS)

clean: 
	rm -f $(BIN) $(AOBJS) $(COBJS) $(MAINOBJ)

