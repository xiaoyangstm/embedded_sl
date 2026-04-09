# XIAO SAMD21 - BME280 I2C 温湿度气压传感器驱动

## 硬件连接

| BME280 引脚 | XIAO SAMD21 引脚 | SAMD21 GPIO | 说明 |
|-------------|-----------------|-------------|------|
| SDA | D4 | PA08 | I2C 数据线 |
| SCL | D5 | PA09 | I2C 时钟线 |
| VCC | 3.3V | - | 供电（勿接 5V） |
| GND | GND | - | 地线 |

BME280 的 I2C 地址为 **0x76**（SDO 接 GND 时）或 **0x70**（SDO 接 VDD 时），本工程使用 0x76。

---

## 整体流程

```
setup()
  ├─ 1. 初始化串口 Serial1 (230400 baud)
  ├─ 2. 初始化 I2C 总线 Wire (100 kHz)
  ├─ 3. 检测 BME280 是否存在（读 Chip ID）
  └─ 4. 初始化 BME280
       ├─ 软复位
       ├─ 等待 NVM 校准数据加载完成
       ├─ 读取出厂校准参数
       └─ 配置工作模式（Normal mode, 过采样 x1）

loop()
  └─ 每秒读取一次传感器数据
       ├─ 从寄存器 0xF7~0xFE 突发读取 8 字节原始 ADC 数据
       ├─ 拼接 20-bit 气压、20-bit 温度、16-bit 湿度
       ├─ 用校准参数做补偿计算
       └─ 串口输出结果
```

---

## 第一步：I2C 基础通信

I2C 是一种主从式总线协议，主机（SAMD21）通过发送设备地址 + 寄存器地址来读写从机（BME280）的数据。

### 1.1 写单个寄存器

向 BME280 的某个寄存器写入一个字节：

```c
static void i2c_write_reg(uint8_t reg, uint8_t val) {
    Wire.beginTransmission(BME280_ADDR);  // 发起通信，发送从机地址
    Wire.write(reg);                       // 写入目标寄存器地址
    Wire.write(val);                       // 写入数据
    Wire.endTransmission();                // 发送 STOP 结束通信
}
```

**I2C 总线上的实际数据流：**
```
[S] 0x76+W  reg  val  [P]
```
- `[S]` = START 条件
- `0x76+W` = 设备地址 + 写方向位
- `reg` = 寄存器地址
- `val` = 数据
- `[P]` = STOP 条件

### 1.2 读单个寄存器

从 BME280 的某个寄存器读取一个字节：

```c
static uint8_t i2c_read_reg(uint8_t reg) {
    Wire.beginTransmission(BME280_ADDR);  // 发起通信
    Wire.write(reg);                       // 发送要读的寄存器地址
    Wire.endTransmission(false);           // 发送 RESTART（不是 STOP），保持总线占用
    Wire.requestFrom((uint8_t)BME280_ADDR, (uint8_t)1);  // 请求读取 1 字节
    return Wire.read();                    // 读取并返回数据
}
```

**I2C 总线上的实际数据流：**
```
[S] 0x76+W  reg  [Sr] 0x76+R  data  [P]
```
- `[Sr]` = 重复 START（RESTART），切换方向从写变读
- `0x76+R` = 设备地址 + 读方向位

### 1.3 突发读取多个寄存器（Burst Read）

从一个起始寄存器开始连续读取多个字节。I2C 主机不发送 STOP，从机会自动递增寄存器地址：

```c
static void i2c_read_regs(uint8_t reg, uint8_t *buf, uint8_t len) {
    Wire.beginTransmission(BME280_ADDR);
    Wire.write(reg);                       // 发送起始寄存器地址
    Wire.endTransmission(false);           // RESTART，不释放总线
    Wire.requestFrom((uint8_t)BME280_ADDR, len);  // 请求读取 len 字节
    for (uint8_t i = 0; i < len; i++) {
        buf[i] = Wire.read();              // 逐字节读取到缓冲区
    }
}
```

**I2C 总线上的实际数据流（以读取 8 字节为例）：**
```
[S] 0x76+W  reg  [Sr] 0x76+R  data0  data1  ...  data7  [P]
```

> **为什么用 `endTransmission(false)`？** 参数 `false` 表示发送 RESTART 而不是 STOP。这是 I2C "复合传输"的标准做法：先写寄存器地址，再切换到读模式读数据，中间不释放总线，避免其他主机抢占总线。

---

## 第二步：检测传感器（读 Chip ID）

BME280 出厂时在寄存器 `0xD0` 写入了固定值 `0x60`，读这个寄存器就能确认传感器是否存在：

```c
static bool bme280_check(void) {
    uint8_t id = i2c_read_reg(0xD0);
    if (id == 0x60) {
        // 找到了
        return true;
    }
    // 没找到，检查接线
    return false;
}
```

**为什么需要这一步？** 在实际项目中，传感器可能没焊好、地址配错、线路断了。读 Chip ID 是最简单的诊断手段——如果连 ID 都读不到，后面所有操作都没有意义。

---

## 第三步：初始化 BME280

初始化分为 4 个子步骤：

### 3.1 软复位

```c
i2c_write_reg(0xE0, 0xB6);  // 向复位寄存器写入魔法值 0xB6
delay(10);                    // 等待复位完成
```

向寄存器 `0xE0` 写入 `0xB6` 会触发芯片软复位，所有配置恢复默认值。这是初始化的好习惯，确保芯片从一个已知状态开始工作。

### 3.2 等待 NVM 校准数据就绪

```c
uint8_t status;
do {
    status = i2c_read_reg(0xF3);  // 读状态寄存器
} while (status & 0x01);          // bit0 = 1 表示正在更新，等待它变为 0
```

BME280 复位后会从内部 NVM 把出厂校准参数拷贝到寄存器映射区。状态寄存器 `0xF3` 的 bit0 表示这个拷贝是否完成。必须等它完成才能读取校准数据。

### 3.3 读取出厂校准参数

BME280 在出厂时每颗芯片都经过了单独校准，校准系数存储在芯片内部 NVM 中。每次上电后需要读出来用于后续的数据补偿。

校准参数分布在两片不连续的寄存器区域：

| 参数 | 寄存器地址 | 字节数 | 类型 |
|------|-----------|--------|------|
| dig_T1~T3 | 0x88~0x8D | 6 | 温度补偿 |
| dig_P1~P9 | 0x8E~0x9F | 18 | 气压补偿 |
| dig_H1 | 0xA1 | 1 | 湿度补偿 |
| dig_H2~H6 | 0xE1~0xE7 | 7 | 湿度补偿 |

读取代码：

```c
static void bme280_read_calibration(void) {
    uint8_t calib_tp[24];
    uint8_t calib_h[7];

    // 一次性读取 0x88~0x9F 共 24 字节（包含温度 + 气压校准参数）
    i2c_read_regs(0x88, calib_tp, 24);

    // 小端序解析：低字节在前，高字节在后
    calib.dig_t1 = (uint16_t)((calib_tp[1] << 8) | calib_tp[0]);
    calib.dig_t2 = (int16_t)((calib_tp[3] << 8) | calib_tp[2]);
    // ... 中间省略，每两个字节拼成一个 16-bit 有/无符号整数

    // dig_H1 单独在 0xA1
    calib.dig_h1 = i2c_read_reg(0xA1);

    // dig_H2~H6 在 0xE1~0xE7
    i2c_read_regs(0xE1, calib_h, 7);
    // ... 解析湿度校准参数
}
```

**注意事项：**
- BME280 的寄存器是**小端序**（Little-Endian），低字节在前，高字节在后
- dig_T1、dig_P1 是 `uint16_t`（无符号），其余大多是 `int16_t`（有符号）
- dig_H4 和 dig_H5 的解析比较特殊，因为它们跨越了非对齐的字节边界：
  ```c
  // dig_H4 = calib_h[3] 的高 4 位 × 16 + calib_h[4] 的低 4 位
  calib.dig_h4 = (int16_t)((int16_t)((int8_t)calib_h[3]) * 16 + (calib_h[4] & 0x0F));
  // dig_H5 = calib_h[5] 的高 4 位 × 16 + calib_h[4] 的高 4 位
  calib.dig_h5 = (int16_t)(((int16_t)((int8_t)calib_h[5]) * 16) | ((calib_h[4] >> 4) & 0x0F));
  ```

### 3.4 配置工作模式

BME280 有三个控制寄存器需要按顺序写入：

```
CTRL_HUM (0xF2) → CTRL_MEAS (0xF4) → CONFIG (0xF5)
```

**注意：** CTRL_HUM 必须在 CTRL_MEAS 之前写入，因为写 CTRL_MEAS 时芯片才会真正刷新内部湿度设置。

```c
// 湿度过采样 = x1（最低精度，但速度最快）
i2c_write_reg(0xF2, 0x01);

// 温度过采样 x1 + 气压过采样 x1 + Normal mode
// bit[7:5] = osrs_t = 001 (温度 x1)
// bit[4:2] = osrs_p = 001 (气压 x1)
// bit[1:0] = mode   = 11  (Normal mode，连续测量)
i2c_write_reg(0xF4, 0x27);

// standby 0.5ms + IIR filter off
// bit[7:5] = t_sb = 000 (standby 0.5ms)
// bit[4:2] = filter = 000 (关闭 IIR 滤波)
// bit(0)   = spi3w_en = 0 (不启用 SPI 3-wire)
i2c_write_reg(0xF5, 0x00);
```

**过采样（Oversampling）是什么？**

ADC 对同一信号多次采样取平均，次数越多精度越高但速度越慢：

| 设置值 | 过采样次数 | ADC 分辨率 |
|-------|-----------|-----------|
| 000 | 跳过（不测量） | - |
| 001 | x1 | 16 bit |
| 010 | x2 | 17 bit |
| 011 | x4 | 18 bit |
| 100 | x8 | 19 bit |
| 101/110/111 | x16 | 20 bit |

**Normal mode vs Forced mode：**
- **Normal mode**：芯片自动连续测量，数据随时可读。适合本项目这种周期读取的场景。
- **Forced mode**：芯片执行一次测量后自动进入 sleep。适合低功耗场景。

---

## 第四步：读取原始 ADC 数据

BME280 的测量数据从寄存器 `0xF7` 开始连续存放，共 8 字节：

```
0xF7  press_msb    气压最高字节
0xF8  press_lsb    气压中间字节
0xF9  press_xlsb   气压最低字节（高 4 位有效）
0xFA  temp_msb     温度最高字节
0xFB  temp_lsb     温度中间字节
0xFC  temp_xlsb    温度最低字节（高 4 位有效）
0xFD  hum_msb      湿度高字节
0xFE  hum_lsb      湿度低字节
```

```c
uint8_t data[8];
i2c_read_regs(0xF7, data, 8);  // 一次性读取 8 字节
```

### 4.1 拼接 20-bit 气压原始值

```c
int32_t adc_press = (int32_t)(((uint32_t)data[0] << 12) |
                              ((uint32_t)data[1] << 4)  |
                              ((uint32_t)data[2] >> 4));
```

分解：
```
data[0] << 12  →  MSB 放到 bit[19:12]
data[1] << 4   →  LSB 放到 bit[11:4]
data[2] >> 4   →  XLSB 的高 4 位放到 bit[3:0]
```
结果是一个 20-bit 无符号整数，范围 0 ~ 1048575。

### 4.2 拼接 20-bit 温度原始值

```c
int32_t adc_temp = (int32_t)(((uint32_t)data[3] << 12) |
                             ((uint32_t)data[4] << 4)  |
                             ((uint32_t)data[5] >> 4));
```

与气压拼接方式完全相同，只是从 data[3] 开始。

### 4.3 拼接 16-bit 湿度原始值

```c
int32_t adc_hum = (int32_t)(((uint16_t)data[6] << 8) | (uint16_t)data[7]);
```

湿度只有 16 bit（没有 xlsb），最简单，就是两个字节拼接。

---

## 第五步：补偿计算

传感器输出的原始 ADC 值不能直接使用，必须用出厂校准参数做补偿计算。这是 BME280 驱动最关键也最容易出错的部分。

### 5.1 温度补偿（必须第一个计算）

```c
static int32_t compensate_temp(int32_t adc_temp) {
    int32_t var1, var2;
    var1 = ((((adc_temp >> 3) - ((int32_t)calib.dig_t1 << 1))) *
            ((int32_t)calib.dig_t2)) >> 11;
    var2 = (((((adc_temp >> 4) - ((int32_t)calib.dig_t1)) *
              ((adc_temp >> 4) - ((int32_t)calib.dig_t1))) >> 12) *
            ((int32_t)calib.dig_t3)) >> 14;
    return var1 + var2;  // 返回的是 t_fine，不是最终温度
}
```

**关键点：** 这个函数返回的不是温度值，而是 **t_fine**（中间变量）。t_fine 是温度的精细表示，后续的气压和湿度补偿都依赖它。这就是为什么温度必须第一个计算。

**计算最终温度（°C × 100）：**
```c
int32_t temp = (t_fine * 5 + 128) >> 8;
// temp 的单位是 0.01°C，除以 100 得到 °C
```

### 5.2 气压补偿

```c
static uint32_t compensate_press(int32_t adc_press, int32_t t_fine) {
    int64_t var1, var2, p;

    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)calib.dig_p6;
    var2 = var2 + ((var1 * (int64_t)calib.dig_p5) << 17);
    var2 = var2 + (((int64_t)calib.dig_p4) << 35);
    var1 = ((var1 * var1 * (int64_t)calib.dig_p3) >> 8) +
           ((var1 * (int64_t)calib.dig_p2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)calib.dig_p1) >> 33;

    if (var1 == 0) return 0;  // 避免除零

    p = 1048576 - (int64_t)adc_press;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)calib.dig_p9) * ((int64_t)(((p >> 13) * (p >> 13)) >> 13))) >> 25;
    var2 = (((int64_t)calib.dig_p8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)calib.dig_p7) << 4);

    return (uint32_t)p;
}
```

**输出格式：** 返回值是 **Q24.8 定点格式**（即实际 Pa 值 × 256）。

转换为 hPa：
```c
float hPa = press / 256.0f / 100.0f;   // 先除 256 得到 Pa，再除 100 得到 hPa
```

> **常见错误：** 直接除以 100 会得到比实际大 ~250 倍的数值。这是因为 Q24.8 格式是 BME280 64 位整数补偿公式的固定输出格式，必须先除以 256。

### 5.3 湿度补偿

```c
static uint32_t compensate_humidity(int32_t adc_hum, int32_t t_fine) {
    int32_t v_x1 = t_fine - ((int32_t)76800);
    v_x1 = ((((adc_hum << 14) - ((int32_t)calib.dig_h4 << 20) -
              ((int32_t)calib.dig_h5 * v_x1)) + ((int32_t)16384)) >> 15) *
             // ... 略
    v_x1 = (v_x1 < 0) ? 0 : v_x1;              // 钳位到 0%
    v_x1 = (v_x1 > 419430400) ? 419430400 : v_x1; // 钳位到 100%
    return (uint32_t)(v_x1 >> 12);
}
```

**输出格式：** 返回值的单位是 **0.001%**（即 % × 1000）。

转换为 %：
```c
float humidity = hum / 1000.0f;
```

---

## 第六步：主程序流程

```c
void setup() {
    Serial1.begin(230400);     // 初始化串口（D6=TX, D7=RX）
    Wire.begin();              // 初始化 I2C（D4=SDA, D5=SCL）
    Wire.setClock(100000);     // 100 kHz Standard Mode

    bme280_check();            // 第 2 步：检测传感器
    bme280_init();             // 第 3 步：初始化
}

void loop() {
    read_sensor();             // 第 4~5 步：读取 + 补偿
    delay(1000);               // 每秒读一次
}
```

---

## 关键寄存器速查表

| 地址 | 名称 | 读/写 | 说明 |
|------|------|-------|------|
| 0xD0 | chip_id | R | 固定值 0x60，用于检测传感器 |
| 0xE0 | reset | W | 写入 0xB6 触发软复位 |
| 0xF2 | ctrl_hum | R/W | 湿度过采样设置 |
| 0xF3 | status | R | bit0: NVM 更新中, bit3: 测量中 |
| 0xF4 | ctrl_meas | R/W | 温度/气压过采样 + 工作模式 |
| 0xF5 | config | R/W | standby 时间 + IIR 滤波器 |
| 0xF7~0xF9 | press | R | 气压原始数据（3 字节） |
| 0xFA~0xFC | temp | R | 温度原始数据（3 字节） |
| 0xFD~0xFE | hum | R | 湿度原始数据（2 字节） |
| 0x88~0xA1 | calib tp/h1 | R | 温度 + 气压 + dig_H1 校准参数 |
| 0xE1~0xE7 | calib h | R | 湿度校准参数 dig_H2~H6 |

---

## 编译与烧录

```bash
# 编译
pio run

# 烧录
pio run --target upload

# 查看串口输出
pio device monitor -b 230400
```

## 串口输出示例

```
============================================
  I2C BME280 Reader - XIAO SAMD21
  SDA: D4 (PA08), SCL: D5 (PA09)
============================================
BME280 found, Chip ID: 0x60
Calibration data loaded
BME280 initialized OK
Starting readings...
Temp: 28.82 C, Press: 990.75 hPa, Hum: 66.35 %
Temp: 28.81 C, Press: 990.72 hPa, Hum: 66.34 %
```
