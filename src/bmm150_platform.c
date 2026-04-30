/**
 * @file bmm150_platform.c
 * @brief BMM150 平台抽象层实现 - STM32F103C8T6 HAL库
 */

#include "bmm150_platform.h"
#include "main.h"       /* STM32 HAL头文件 */

/* 外部变量声明 - 由CubeMX生成 */
extern I2C_HandleTypeDef hi2c1;

/**
 * @brief 读取寄存器
 */
bmm150_status_t bmm150_read_reg(bmm150_dev_t *dev, uint8_t reg_addr, uint8_t *data, uint16_t len)
{
    HAL_StatusTypeDef status;

    if (dev == NULL || data == NULL) {
        return BMM150_E_NULL_PTR;
    }

    status = HAL_I2C_Mem_Read(&hi2c1,
                              (uint16_t)(dev->i2c_addr << 1),
                              reg_addr,
                              I2C_MEMADD_SIZE_8BIT,
                              data,
                              len,
                              BMM150_I2C_TIMEOUT);

    if (status == HAL_OK) {
        return BMM150_OK;
    } else if (status == HAL_TIMEOUT) {
        return BMM150_E_NOT_READY;
    } else {
        return BMM150_E_COMM_FAIL;
    }
}

/**
 * @brief 写入寄存器
 */
bmm150_status_t bmm150_write_reg(bmm150_dev_t *dev, uint8_t reg_addr, uint8_t *data, uint16_t len)
{
    HAL_StatusTypeDef status;

    if (dev == NULL || data == NULL) {
        return BMM150_E_NULL_PTR;
    }

    status = HAL_I2C_Mem_Write(&hi2c1,
                               (uint16_t)(dev->i2c_addr << 1),
                               reg_addr,
                               I2C_MEMADD_SIZE_8BIT,
                               data,
                               len,
                               BMM150_I2C_TIMEOUT);

    if (status == HAL_OK) {
        return BMM150_OK;
    } else if (status == HAL_TIMEOUT) {
        return BMM150_E_NOT_READY;
    } else {
        return BMM150_E_COMM_FAIL;
    }
}

/**
 * @brief 毫秒级延时
 */
void bmm150_delay_ms(uint32_t ms)
{
    HAL_Delay(ms);
}