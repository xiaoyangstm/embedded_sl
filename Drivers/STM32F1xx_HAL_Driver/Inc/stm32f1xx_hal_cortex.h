/**
 * @file stm32f1xx_hal_cortex.h
 * @brief HAL Cortex Header
 */

#ifndef __STM32F1XX_HAL_CORTEX_H
#define __STM32F1XX_HAL_CORTEX_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal_def.h"

/* Function Declarations */
void HAL_NVIC_SetPriority(uint8_t IRQn, uint32_t PriorityGroup, uint32_t PreemptPriority, uint32_t SubPriority);
void HAL_NVIC_EnableIRQ(uint8_t IRQn);
void HAL_NVIC_DisableIRQ(uint8_t IRQn);
void HAL_SYSTICK_Config(uint32_t TicksNumb);

#ifdef __cplusplus
}
#endif

#endif /* __STM32F1XX_HAL_CORTEX_H */