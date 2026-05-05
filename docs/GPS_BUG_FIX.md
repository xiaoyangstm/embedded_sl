# GPS经纬度数据解析Bug修复文档

## 问题描述

GPS模块(ATGM336H)能够正常接收卫星信号(显示15-19颗卫星)，但经纬度数据始终为空，无法正确解析和显示。

## 问题现象

```
GPS: lat=, lon=, sats=17
Upload: {"gyroX":13,"gyroY":-175,"gyroZ":-100,"magneticFlux":201,"lat":,"lon":,"sats":17}
```

卫星数量正常显示，但经纬度字段为空。

## 根本原因

**STM32使用newlib-nano库，默认不支持浮点数格式化输出（`%f`、`%.6f`等）**

当使用 `sprintf` 或 `snprintf` 配合 `%.6f` 格式化浮点数时，输出为空字符串。这是因为：
1. PlatformIO默认使用 `-specs=nano.specs` 链接选项
2. nano库为了节省Flash空间，移除了浮点数格式化支持
3. 即使浮点数计算正确，`sprintf(buf, "%.6f", value)` 也会输出空字符串

## 解决方案

### 方案一：移除nano.specs（启用完整标准库）

修改 `platformio.ini`：
```ini
[env:bluepill_f103c8]
platform = ststm32
board = bluepill_f103c8
framework = stm32cube

build_flags =
    -I NET/inc

; 移除nano.specs以支持浮点数打印
build_unflags = -specs=nano.specs
```

**优点**：简单直接，支持所有标准库功能
**缺点**：增加约5-10KB Flash占用

### 方案二：使用整数运算（最终采用）

将GPS坐标从浮点数改为整数存储，避免浮点数格式化问题。

#### 1. 修改数据结构 (`gps.h`)

```c
/* GPS数据结构 */
typedef struct {
    int32_t latitude;        /* 纬度 (微度, 实际值 = latitude / 1000000) */
    int32_t longitude;       /* 经度 (微度, 实际值 = longitude / 1000000) */
    uint8_t valid;           /* 数据有效标志 */
    uint8_t sat_count;       /* 卫星数量 */
} gps_data_t;
```

#### 2. 实现整数坐标转换函数 (`gps.c`)

```c
/**
 * @brief 将NMEA格式的坐标转换为十进制度数（整数版本）
 * NMEA格式: ddmm.mmmm 或 dddmm.mmmm
 * 返回: 整数形式的十进制度数 * 1000000 (微度)
 */
static int32_t Convert_to_degrees_int(char* data)
{
    if (data == NULL || strlen(data) == 0) return 0;

    /* 手动解析浮点数 */
    int32_t raw = 0;
    int i = 0;
    int32_t sign = 1;

    while (data[i] == ' ') i++;

    if (data[i] == '-') { sign = -1; i++; }
    else if (data[i] == '+') { i++; }

    /* 解析整数部分 */
    while (data[i] >= '0' && data[i] <= '9') {
        raw = raw * 10 + (data[i] - '0');
        i++;
    }

    /* 解析小数部分 (最多4位) */
    int32_t fraction = 0;
    int frac_digits = 0;
    if (data[i] == '.') {
        i++;
        while (data[i] >= '0' && data[i] <= '9' && frac_digits < 4) {
            fraction = fraction * 10 + (data[i] - '0');
            frac_digits++;
            i++;
        }
        while (frac_digits < 4) {
            fraction *= 10;
            frac_digits++;
        }
    }

    int32_t full_value = raw * 10000 + fraction;
    full_value *= sign;

    /* 提取度数和分钟 */
    int32_t degrees = full_value / 1000000;
    int32_t minutes_full = full_value - degrees * 1000000;
    int32_t minutes = minutes_full / 10000;
    int32_t minutes_frac = minutes_full % 10000;

    /* 转换为十进制度数 (微度) */
    int32_t result = degrees * 1000000;
    result += minutes * 16666;
    result += minutes_frac * 1666 / 10000;

    return result;
}
```

#### 3. 修改JSON输出格式 (`onenet.c`)

```c
/* GPS latitude - 整数格式 */
memset(text, 0, sizeof(text));
sprintf(text, "\"lat\":%ld,", gps->latitude);
strcat(buf, text);

/* GPS longitude - 整数格式 */
memset(text, 0, sizeof(text));
sprintf(text, "\"lon\":%ld,", gps->longitude);
strcat(buf, text);
```

## 数据格式说明

### NMEA坐标格式
- 纬度: `2859.53188` 表示 28°59.53188' = 28.992198°
- 经度: `11140.63440` 表示 111°40.63440' = 111.672240°

### 整数存储格式
- 存储值 = 实际度数 × 1000000
- 例如: 28.992198° 存储为 28992198
- 解析时: 实际值 = 存储值 / 1000000

## 修复后输出示例

```
GPS: lat=28984179, lon=111667696, sats=17, valid=1
Upload: {"gyroX":13,"gyroY":-175,"gyroZ":-100,"magneticFlux":201,"lat":28984179,"lon":111667696,"sats":17}
```

## 经验总结

1. **嵌入式开发中避免使用浮点数格式化**：nano库默认不支持，应使用整数运算
2. **调试时注意区分问题层级**：数据解析正确 ≠ 数据显示正确
3. **使用 `%ld` 格式化 `int32_t`**：避免类型不匹配问题
4. **手动实现字符串转数值**：避免依赖标准库函数可能存在的限制

## 文件修改清单

| 文件 | 修改内容 |
|------|----------|
| `NET/inc/gps.h` | 修改 `gps_data_t` 结构体，使用 `int32_t` 存储坐标 |
| `NET/src/gps.c` | 新增 `Convert_to_degrees_int()` 函数，移除调试输出 |
| `NET/src/onenet.c` | 修改JSON格式化，使用 `%ld` 输出整数坐标 |
| `src/main.c` | 修改GPS数据显示格式 |
| `platformio.ini` | 添加 `build_unflags = -specs=nano.specs` |
