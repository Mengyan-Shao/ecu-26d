# HRT 26D ECU

HRT 赛车队 26D 赛季整车 ECU 固件。

STM32F1 平台（CMSIS 使用 `stm32f10x_cl` 启动文件）+ ST 标准外设库，Keil MDK5 工程。

## 功能模块

| 模块 | 文件 | 说明 |
| --- | --- | --- |
| 主程序 | `User/main.c` | 初始化 + 10 ms 周期任务调度 |
| 10 ms 任务 | `User/stm32f10x_it.c` → `ECU_Task_10ms()` | 整车状态机主逻辑 |
| CAN 通信 | `User/CAN.c` | 整车报文收发（`HRT_CTRL` 0x156 等） |
| 串口 | `User/USART.c` | 调试输出与 SBUS 接收 |
| 定时器 | `User/TIME.c` | 系统时基 |
| GPIO | `User/GPIO.c` | 指示灯与使能信号 |
| 状态标志 | `User/Flag.c` | 全局状态位管理 |

## 硬件

- MCU：STM32F105RB
- SBUS 接收机 → `USART3_RX`（PB11），100000 baud / 9 数据位 / 偶校验 / 2 停止位
- CAN 接入整车网络

## 目录结构

```
ECU20263.1/
├── ECU26DVersionFinal/
│   ├── CMSIS/     # 内核与启动文件
│   ├── FWLIB/     # ST 标准外设库
│   ├── Project/   # Keil 工程（ASECU.uvprojx）
│   └── User/      # 业务代码
├── docs/images/   # 文档配图
└── 遥控器.md       # SBUS 遥控功能说明
```

## 构建

用 Keil MDK5 打开 `ECU26DVersionFinal/Project/ASECU.uvprojx`，编译 target `无人ECU`。

> 编译产物（`Objects/`、`Listings/`）和 Keil 个人配置（`*.uvoptx`、`*.uvguix.*`）已在 `.gitignore` 中排除，克隆后需重新编译。

> 仓库内源码统一为 **UTF-8（带 BOM）**。若在 Keil 中打开中文注释显示为乱码，把 `Edit → Configuration → Editor → Encoding` 改成 `UTF-8` 即可。

## 文档

- [遥控器使用说明（SBUS）](遥控器.md)

## 说明

本项目为车队内部工程代码，版权归 HRT 赛车队所有。未经许可，不得用于其他参赛车辆或商业用途。
