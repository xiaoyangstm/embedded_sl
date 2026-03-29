# XIAO nRF54L15 双串口测试项目

## 项目简介

这是一个用于测试 Seeed XIAO nRF54L15 开发板双串口功能的 Zephyr RTOS 项目。

## 硬件信息

- **开发板**: Seeed XIAO nRF54L15
- **SDK**: nRF Connect SDK v3.2.1
- **Zephyr**: v4.2.99

### 串口配置

| 串口 | 类型 | 引脚 | 用途 |
|------|------|------|------|
| UART20 | USB 虚拟串口 | USB-C | 连接 PC，用于调试输出 |
| UART21 | GPIO 串口 | D6(TX), D7(RX) | 连接外部设备 |

## 编译和烧录

### 前置要求

- 安装 nRF Connect SDK v3.2.1
- 配置好 Zephyr 开发环境

### 编译

```bash
west build -b xiao_nrf54l15/nrf54l15/cpuapp
```

### 烧录

```bash
west flash
```

### 查看串口输出

```bash
# 查看控制台输出 (UART20)
minicom -D /dev/ttyACM0 -b 115200
```

## 项目结构

```
.
├── CMakeLists.txt           # CMake 构建配置
├── prj.conf                 # 项目配置文件
├── src/
│   └── main.c              # 主程序代码
├── boards/
│   └── xiao_nrf54l15_nrf54l15_cpuapp.overlay  # 设备树覆盖文件
└── README.md               # 本文件
```

## 代码说明

### 设备树配置

`boards/xiao_nrf54l15_nrf54l15_cpuapp.overlay` 文件定义了 UART21 的配置：

- 引脚映射: TX=P2.08(D6), RX=P2.07(D7)
- 波特率: 115200
- 包含 pinctrl 配置

### 主程序

`src/main.c` 实现了双串口数据发送：

1. 使用 `DT_CHOSEN(uart_passthrough)` 获取 UART21 设备
2. 使用 `printk()` 向 USB 串口 (UART20) 输出
3. 使用 `uart_poll_out()` 向 GPIO 串口 (UART21) 输出
4. 添加了设备就绪检查，防止硬件错误

## 测试方法

### USB 串口测试

直接通过 USB-C 线连接电脑，波特率 115200，可以看到：
```
=== UART Test ===
UART21 device pointer: 0x...
UART21 is ready!
=================

[0] USB test
[1] USB test
...
```

### GPIO 串口测试

使用 USB转TTL 模块连接：

```
XIAO D6 (TX) → TTL 模块 RX
XIAO D7 (RX) → TTL 模块 TX
XIAO GND     → TTL 模块 GND
```

波特率设置: 115200

预期输出:
```
ABC
ABC
ABC
...
```

## 故障排除

### 编译错误

如果遇到 `uart_passthrough` 未定义错误，检查 overlay 文件是否正确放置在 `boards/` 目录下。

### 串口无输出

1. 检查设备是否就绪: 查看日志中的 "UART21 is ready"
2. 检查引脚连接: 确认 D6/D7 正确连接
3. 检查波特率: 确保设置为 115200

### HARD FAULT 错误

确保在访问设备前调用 `device_is_ready()` 检查设备状态。

## 技术细节

### 关键宏说明

| 宏 | 作用 |
|-----|------|
| `DT_CHOSEN(name)` | 查找设备树 chosen 节点 |
| `DEVICE_DT_GET(node)` | 获取设备结构体指针 |
| `device_is_ready(dev)` | 检查设备是否已初始化 |

### 为什么使用 uart_poll_out

- `uart_tx()` API 在某些驱动上不被支持（返回 -134 ENOTSUP）
- `uart_fifo_fill()` 需要中断配置
- `uart_poll_out()` 是最可靠的轮询方式，适合简单测试

## 许可证

本项目基于原始 nRF Connect SDK 示例，遵循 Apache-2.0 许可证。

## 参考资源

- [nRF Connect SDK 文档](https://developer.nordicsemi.com/)
- [Zephyr 文档](https://docs.zephyrproject.org/)
- [Seeed Studio XIAO nRF54L15](https://www.seeedstudio.com/)
