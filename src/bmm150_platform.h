/**
 * @file bmm150_platform.h
 * @brief BMM150 平台抽象层 - STM32F103C8T6 I2C接口
 */

#ifndef __BMM150_PLATFORM_H
#define __BMM150_PLATFORM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "bmm150.h"

/* 平台配置 - 根据你的硬件连接修改 */
#define BMM150_I2C_HANDLE       hi2c1       /* I2C句柄 */
#define BMM150_I2C_TIMEOUT      100         /* I2C超时(ms) */

/* 函数声明 - 需要用户实现 */

/**
 * @brief 读取寄存器
 * @param dev 设备句柄
 * @param reg_addr 寄存器地址
 * @param data 数据缓冲区
 * @param len 数据长度
 * @return 状态码
 */
bmm150_status_t bmm150_read_reg(bmm150_dev_t *dev, uint8_t reg_addr, uint8_t *data, uint16_t len);

/**
 * @brief 写入寄存器
 * @param dev 设备句柄
 * @param reg_addr 寄存器地址
 * @param data 数据缓冲区
 * @param len 数据长度
 * @return 状态码
 */
bmm150_status_t bmm150_write_reg(bmm150_dev_t *dev, uint8_t reg_addr, uint8_t *data, uint16_t len);

/**
 * @brief 毫秒级延时
 * @param ms 延时毫秒数
 */
void bmm150_delay_ms(uint32_t ms);

#ifdef __cplusplus
}
#endif

#endif /* __BMM150_PLATFORM_H */