/**
 * @file stm32f1xx_hal_flash.h
 * @brief HAL FLASH Header
 */

#ifndef __STM32F1XX_HAL_FLASH_H
#define __STM32F1XX_HAL_FLASH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal_def.h"

void HAL_FLASH_Unlock(void);
void HAL_FLASH_Lock(void);

#ifdef __cplusplus
}
#endif

#endif /* __STM32F1XX_HAL_FLASH_H */