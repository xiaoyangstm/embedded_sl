/**
 * @file stm32f1xx_hal.h
 * @brief STM32F1xx HAL Main Header
 */

#ifndef __STM32F1XX_HAL_H
#define __STM32F1XX_HAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal_conf.h"

void HAL_Init(void);
void HAL_MspInit(void);
void HAL_Delay(uint32_t Delay);
uint32_t HAL_GetTick(void);
void HAL_IncTick(void);

#ifdef __cplusplus
}
#endif

#endif /* __STM32F1XX_HAL_H */