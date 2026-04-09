# W25Q128JV SPI Flash Test — XIAO nRF54L15

## 项目简介

通过 XIAO nRF54L15 开发板的 SPI 接口读写 W25Q128JV（或 GD25Q128）SPI Flash 芯片，基于 Zephyr RTOS (PlatformIO)。

## 接线

| Flash 引脚 | XIAO 引脚 | nRF54L15 GPIO |
|-----------|-----------|---------------|
| SCK       | D8        | P2.01         |
| MISO      | D9        | P2.04         |
| MOSI      | D10       | P2.02         |
| CS        | D1        | P1.05         |
| WP#       | —         | 拉高          |
| HOLD#     | —         | 拉高          |

## 构建与烧录

```shell
# 编译
pio run

# 烧录
pio run --target upload

# 清理后重新编译（改过 overlay/conf 后建议先 clean）
pio run --target clean && pio run
```

## 踩坑记录

### 问题一：SPI device not ready

**现象：** 烧录后串口输出 `SPI device not ready`，程序直接退出。

**排查过程：**

1. `device_is_ready()` 返回 false 意味着 SPI 控制器驱动初始化失败，与外部 Flash 芯片的物理接线无关
2. 检查设备树（DTS）发现 `spi00` 节点在 SoC DTSI 中缺少 `clocks` 属性，overlay 已补上 `clocks = <&hfpll>`
3. 检查 Kconfig 确认 `CONFIG_SPI_NRFX_SPIM=y` 已生效
4. 添加 `CONFIG_SPI_LOG_LEVEL_DBG=y` 等调试选项后重新编译，SPI 驱动正常初始化

**原因：** 之前的构建缓存可能未正确应用 overlay，重新编译后解决。

**教训：** 改过 overlay 或 prj.conf 后如果行为异常，先尝试 clean build（`pio run --target clean` 后重新编译）。

### 问题二：栈溢出（Stack overflow）

**现象：** SPI 读写成功（JEDEC ID 正确、擦除和写入均 OK），但在回读阶段崩溃：

```
***** USAGE FAULT *****
  Stack overflow (context area not valid)
>>> ZEPHYR FATAL ERROR 2: Stack overflow on CPU 0
```

**原因：** `read_flash()` 函数在栈上分配了两个大数组：

```c
uint8_t tx[4 + 256];  // 260 字节
uint8_t rx[4 + 256];  // 260 字节
```

加上 `main()` 中的局部变量、`dump_hex()` 的 `char line[80]`、SPI 驱动和日志函数的栈需求，总计超过了 Zephyr 主线程默认栈大小（通常 1024 字节）。

**解决方案：**

1. 在 `prj.conf` 中增大主线程栈：

```ini
CONFIG_MAIN_STACK_SIZE=2048
```

2. 将 `read_flash()` 中的大数组改为 `static`，从栈移到 BSS 段：

```c
static uint8_t tx[4 + 256];
static uint8_t rx[4 + 256];
```

**教训：** 在嵌入式开发中，避免在栈上分配大块内存。对于超过几十字节的缓冲区，优先使用 `static` 或全局数组。Zephyr 主线程默认栈较小，需根据实际需求调整。
