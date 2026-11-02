# 养老院监护系统（i.MX6ULL + LVGL）

基于 **LVGL v8.3** 的养老院监护系统，运行于**正点原子阿尔法 i.MX6ULL EMMC 开发板 + 4.3 寸 RGB 屏**（ATK-4342，800×480，电容触摸）。

## 功能

1. **环境监测**：温度 / 湿度 / 光照实时显示（表盘 + 数值 + 超限变色）。
   数据由 STM32 模块采集、通过 WiFi 上报；当前为模拟数据，接口已预留（见下文"对接真实硬件"）。
2. **病人端**：人脸识别 → 体温检测 → 自动记录当日体温（每天只记录一次）。
   人脸识别当前为模拟流程，接口已预留。
3. **管理员后台**：密码登录（默认 `123456`）→ 病人列表（含今日体温状态）→
   查看病人今日检测记录与近 7 天历史 → 填写血糖、血压等检测值。
   数据当前保存在内存中，接口已预留 SQLite 替换。

## 界面流程

```
首页 ── 环境监测（表盘实时数据）
   ├── 病人检测 ── 人脸识别 ── 体温检测 ── 自动记录
   └── 管理员 ── 登录 ── 病人列表 ── 病人详情（今日记录/近7天）
                               └── 填写检测值（血糖/血压，数字键盘）
```

## 目录结构

```
├── main.c            入口（fbdev/evdev 初始化 + UI）
├── app_data.h/.c     数据层（环境/病人/记录；真实硬件的对接点）
├── ui/               UI 界面（首页/环境/病人/管理员/公共组件）
│   ├── lv_font_cjk_16.c  中文字体 16px（wqy-microhei 定制字库）
│   └── lv_font_cjk_24.c  中文字体 24px
├── tools/            字体生成工具
│   ├── make_font.py      UI 文案 -> 字符集（改文案后需重跑）
│   ├── check_font.py     校验字体覆盖
│   └── fonts/wqy-microhei.ttf  开源中文字体（Apache 2.0）
├── lvgl/             LVGL v8.3.11
└── lv_drivers/       fbdev / evdev 驱动
```

## 编译（阿尔法 i.MX6ULL）

```sh
make clean
make CROSS_COMPILE=arm-linux-gnueabihf-   # 换成你的工具链前缀
```

生成可执行文件 `demo`。部署到板子：

```sh
scp demo root@<板子IP>:/root/
# 板上执行 ./demo（需存在 /dev/fb0，权限不足用 root）
```

## 板上必改配置

| 项 | 位置 | 说明 |
|---|---|---|
| 触摸设备号 | `lv_drv_conf.h` 的 `EVDEV_NAME` | 默认 `/dev/input/event10`，用 `evtest` 查到触摸屏实际是 `eventX` 后修改 |
| 屏幕分辨率 | `main.c` 的 `hor_res/ver_res` | 4.3 寸 RGB 屏已配好 800×480；换 7 寸屏改为 1024×600 |
| 系统时间 | 板子 RTC | 记录按日期归档，请先 `date -s "2026-11-01 12:00:00"` 同步时间 |

## 对接真实硬件（预留接口）

- **STM32 环境数据（WiFi 上报）**：接收线程解析数据后调用
  `app_data_env_set(temp, humi, light, wifi_connected)`，然后删除
  `app_data_env_sim()` 的调用（在 `ui_env.c` / `ui_home.c` 的定时器里）。
- **人脸识别**：真实模块返回病人编号后替换 `app_data_face_recognize()` 的实现。
- **数据库**：把 `app_data.c` 中的记录读写函数替换为 SQLite 实现，
  UI 层无需改动。管理员密码在 `ui_admin.c` 的 `ADMIN_PASSWORD`。

## 字体说明

界面中文使用 `wqy-microhei`（文泉驿微米黑，Apache 2.0 协议）生成的
**只含 UI 所需字符**的定制字库，烘焙进二进制，无运行时文件依赖。

新增界面文案后，先改 `tools/make_font.py` 的 STRINGS，再重新生成字体：

```sh
python tools/make_font.py > tools/chars.txt
SYMS="$(cat tools/chars.txt)"
lv_font_conv --font tools/fonts/wqy-microhei.ttf --size 16 --bpp 4 --format lvgl \
    --symbols "$SYMS" -r 0x20-0x7F --output ui/lv_font_cjk_16.c --no-compress
lv_font_conv --font tools/fonts/wqy-microhei.ttf --size 24 --bpp 4 --format lvgl \
    --symbols "$SYMS" -r 0x20-0x7F --output ui/lv_font_cjk_24.c --no-compress
python tools/check_font.py ui/lv_font_cjk_16.c   # 校验全部字符已覆盖
```

（lv_font_conv 安装：`npm install -g lv_font_conv`）
