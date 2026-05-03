/**
 * @file stm32f1xx_hal_rcc.h
 * @brief HAL RCC Header
 */

#ifndef __STM32F1XX_HAL_RCC_H
#define __STM32F1XX_HAL_RCC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal_def.h"

/* Function Declarations */
void HAL_RCC_ClockConfig(void);
void HAL_RCC_OscConfig(void);
uint32_t HAL_RCC_GetPCLK1Freq(void);
uint32_t HAL_RCC_GetPCLK2Freq(void);
uint32_t HAL_RCC_GetSysClockFreq(void);

#ifdef __cplusplus
}
#endif

#endif /* __STM32F1XX_HAL_RCC_H */