/**
 * @file stm32f1xx_hal_i2c.h
 * @brief HAL I2C Header
 */

#ifndef __STM32F1XX_HAL_I2C_H
#define __STM32F1XX_HAL_I2C_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal_def.h"

/* I2C Init Structure */
typedef struct {
  uint32_t ClockSpeed;
  uint32_t DutyCycle;
  uint32_t OwnAddress1;
  uint32_t AddressingMode;
  uint32_t DualAddressMode;
  uint32_t OwnAddress2;
  uint32_t GeneralCallMode;
  uint32_t NoStretchMode;
} I2C_InitTypeDef;

/* I2C Handle Structure */
typedef struct {
  I2C_TypeDef *Instance;
  I2C_InitTypeDef Init;
  HAL_LockTypeDef Lock;
  uint32_t State;
} I2C_HandleTypeDef;

/* I2C Clock Speed */
#define I2C_SPEED_STANDARD         100000
#define I2C_SPEED_FAST             400000

/* I2C Duty Cycle */
#define I2C_DUTYCYCLE_2            0x00000000
#define I2C_DUTYCYCLE_16_9         0x00004000

/* I2C Addressing Mode */
#define I2C_ADDRESSINGMODE_7BIT    0x00004000
#define I2C_ADDRESSINGMODE_10BIT   0x00008000

/* I2C Dual Address */
#define I2C_DUALADDRESS_DISABLE    0x00000000
#define I2C_DUALADDRESS_ENABLE     I2C_OAR2_ENDUAL

/* I2C General Call */
#define I2C_GENERALCALL_DISABLE    0x00000000
#define I2C_GENERALCALL_ENABLE     I2C_CR1_ENGC

/* I2C No Stretch */
#define I2C_NOSTRETCH_DISABLE      0x00000000
#define I2C_NOSTRETCH_ENABLE       I2C_CR1_NOSTRETCH

/* Memory Address Size */
#define I2C_MEMADD_SIZE_8BIT       0x00000001
#define I2C_MEMADD_SIZE_16BIT      0x00000010

/* I2C State */
#define HAL_I2C_STATE_RESET        0x00
#define HAL_I2C_STATE_READY        0x01
#define HAL_I2C_STATE_BUSY         0x02

/* I2C Endual bit */
#define I2C_OAR2_ENDUAL            0x00000001

/* I2C CR1 bits */
#define I2C_CR1_ENGC               0x00000800
#define I2C_CR1_NOSTRETCH          0x00000080

/* Function Declarations */
HAL_StatusTypeDef HAL_I2C_Init(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef HAL_I2C_DeInit(I2C_HandleTypeDef *hi2c);
void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c);
void HAL_I2C_MspDeInit(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef HAL_I2C_Master_Transmit(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout);
HAL_StatusTypeDef HAL_I2C_Master_Receive(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout);
HAL_StatusTypeDef HAL_I2C_Mem_Write(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout);
HAL_StatusTypeDef HAL_I2C_Mem_Read(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout);
HAL_StatusTypeDef HAL_I2C_IsDeviceReady(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint32_t Trials, uint32_t Timeout);

#ifdef __cplusplus
}
#endif

#endif /* __STM32F1XX_HAL_I2C_H */