<<<<<<< HEAD
# BMM150 三轴磁力计 - STM32F103C8T6 + ESP8266 MQTT

基于 STM32F103C8T6 的 BMM150 三轴地磁传感器驱动，通过 ESP8266 WiFi 模块将数据上传到 MQTT 服务器。

## 功能特性

- BMM150 三轴磁力计驱动（I2C 接口）
- ESP8266 WiFi 连接
- MQTT 协议数据上传
- 实时磁场数据监测
- 支持远程 LED 控制

## 项目结构

```
BMM150/
├── src/                          # 主程序源码
│   ├── main.c                    # 主程序入口
│   ├── bmm150.c                  # BMM150 驱动实现
│   ├── bmm150.h                  # BMM150 驱动头文件
│   ├── bmm150_platform.c         # 平台抽象层实现
│   ├── bmm150_platform.h         # 平台抽象层头文件
│   ├── stm32f1xx_hal_msp.c       # HAL MSP 初始化
│   └── stm32f1xx_it.c            # 中断处理
├── NET/                          # 网络通信模块
│   ├── inc/                      # 头文件
│   │   ├── esp8266.h             # ESP8266 驱动
│   │   ├── onenet.h              # OneNET/MQTT 接口
│   │   ├── MqttKit.h             # MQTT 协议封装
│   │   └── cJSON.h               # JSON 解析库
│   ├── src/                      # 源文件
│   │   ├── esp8266.c
│   │   ├── onenet.c
│   │   ├── MqttKit.c
│   │   └── cJSON.c
│   └── library.json              # PlatformIO 库配置
├── Drivers/                      # STM32 HAL 驱动库
├── platformio.ini                # PlatformIO 配置
└── README.md
```

## 硬件要求

### 主控板
- STM32F103C8T6 (Blue Pill)
- 72MHz 主频
- 64KB Flash / 20KB RAM

### 传感器模块
- BMM150 三轴磁力计模块（微雪或其他兼容模块）
- I2C 接口

### 通信模块
- ESP8266 WiFi 模块（ESP-01/ESP-12 等）

## 硬件连接

### BMM150 磁力计

| BMM150 引脚 | STM32 引脚 | 说明 |
|------------|-----------|------|
| VCC        | 3.3V      | 电源 |
| GND        | GND       | 地 |
| SCL        | PB6       | I2C1_SCL |
| SDA        | PB7       | I2C1_SDA |
| INT        | 不接      | 中断（可选） |

### ESP8266 WiFi 模块

| ESP8266 引脚 | STM32 引脚 | 说明 |
|-------------|-----------|------|
| VCC         | 3.3V      | 电源 |
| GND         | GND       | 地 |
| TX          | PA3       | USART2_RX |
| RX          | PA2       | USART2_TX |
| EN          | 3.3V      | 使能 |
| RST         | 3.3V      | 复位 |

### 调试串口

| 串口 | STM32 引脚 | 说明 |
|-----|-----------|------|
| TX  | PA9       | USART1_TX |
| RX  | PA10      | USART1_RX |

### LED 指示灯

| LED | STM32 引脚 |
|-----|-----------|
| LED | PC13      |

## 软件配置

### WiFi 配置

在 `NET/src/esp8266.c` 中修改：

```c
#define WIFI_SSID     "YourWiFiName"     // WiFi 名称
#define WIFI_PASSWORD "YourWiFiPassword" // WiFi 密码
```

### MQTT 服务器配置

在 `NET/src/onenet.c` 中修改：

```c
#define DEVICE_NAME    "bmm150_device"   // 设备名称
#define MQTT_USER      "bmm150"          // MQTT 用户名
#define MQTT_PASSWORD  "bmm150123"       // MQTT 密码
```

### MQTT 服务器地址

在 `NET/src/esp8266.c` 中修改：

```c
#define MQTT_SERVER    "your_mqtt_server_ip"  // MQTT 服务器 IP
#define MQTT_PORT      "1883"                  // MQTT 端口
```

## 编译与烧录

### 环境要求

- PlatformIO (推荐) 或 STM32CubeIDE
- ST-Link 调试器

### 使用 PlatformIO 编译

```bash
# 编译
pio run

# 编译并烧录
pio run -t upload

# 清理
pio run -t clean

# 查看串口输出
pio device monitor -b 115200
```

### 使用 ST-Link 烧录

```bash
st-flash write .pio/build/bluepill_f103c8/firmware.bin 0x08000000
```

## 数据格式

### 上传数据 (JSON)

```json
{
  "gyroX": 88,
  "gyroY": -30,
  "gyroZ": 25,
  "magneticFlux": 96
}
```

| 字段 | 说明 | 单位 |
|------|------|------|
| gyroX | X 轴磁场分量 | LSB |
| gyroY | Y 轴磁场分量 | LSB |
| gyroZ | Z 轴磁场分量 | LSB |
| magneticFlux | 磁场总强度 | LSB |

### 下发控制 (JSON)

```json
{
  "led": 1
}
```

| 字段 | 值 | 说明 |
|------|-----|------|
| led | 1 | LED 亮 |
| led | 0 | LED 灭 |

## 串口调试输出

默认波特率：115200

```
USART1 Test OK!
System Init...
USART2 Init OK
I2C1 Init OK
Scanning I2C bus...
Found device at 0x26 (7-bit: 0x13)
Scan complete.
Initializing BMM150...
Chip ID after wake: 0x32 (expected: 0x32)
BMM150 Init OK

=================================
STM32F103C8T6 @ 72MHz
BMM150 + ESP8266 MQTT
=================================

Initializing ESP8266...
1. AT Test
   AT OK
2. Set Station Mode
   CWMODE OK
...
MQTT Connected OK

Start reading BMM150 sensor data...

[0] X:88 Y:-30 Z:25 (   uT)
Upload: {"gyroX":88,"gyroY":-30,"gyroZ":25,"magneticFlux":96}
MQTT Publish OK
```

## BMM150 API 参考

### 初始化

```c
bmm150_dev_t bmm150_dev;
bmm150_status_t status = bmm150_init(&bmm150_dev, BMM150_DEFAULT_I2C_ADDRESS);
```

### 读取数据

```c
bmm150_data_t mag_data;
bmm150_get_mag_data(&bmm150_dev, &mag_data);

// 原始数据
int16_t x = mag_data.x;
int16_t y = mag_data.y;
int16_t z = mag_data.z;

// 微特斯拉值
float x_uT = mag_data.x_uT;
float y_uT = mag_data.y_uT;
float z_uT = mag_data.z_uT;
```

### 设置操作模式

```c
// 正常模式（持续测量）
bmm150_set_op_mode(&bmm150_dev, BMM150_NORMAL_MODE);

// 强制模式（单次测量）
bmm150_set_op_mode(&bmm150_dev, BMM150_FORCED_MODE);

// 睡眠模式
bmm150_set_op_mode(&bmm150_dev, BMM150_SLEEP_MODE);
```

### 设置精度

```c
// 高精度模式
bmm150_set_xy_rep(&bmm150_dev, BMM150_REPXY_HIGHACCURACY);
bmm150_set_z_rep(&bmm150_dev, BMM150_REPZ_HIGHACCURACY);
```

## 常见问题

### 1. BMM150 初始化失败

- 检查 I2C 接线（SCL/SDA 是否接反）
- 确认供电电压为 3.3V
- 运行 I2C 扫描确认设备地址

### 2. WiFi 连接失败

- 确认 WiFi 名称和密码正确
- 检查 ESP8266 供电是否稳定
- 确认 ESP8266 固件支持 AT 指令

### 3. MQTT 连接失败

- 确认 MQTT 服务器地址和端口正确
- 检查用户名和密码
- 确认服务器允许客户端连接

### 4. 数据异常

- 远离铁磁性材料
- 执行传感器校准
- 检查是否有强磁场干扰

## 注意事项

1. BMM150 测量的是地磁场，需远离铁磁性材料和强磁场
2. 实际应用中需要进行硬铁/软铁校准
3. 地球磁场强度约为 25-65 μT
4. ESP8266 需要稳定的 3.3V 电源，峰值电流可达 300mA

## 许可证

MIT License
