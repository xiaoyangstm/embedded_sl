/**
 * @file stm32f1xx_hal_def.h
 * @brief HAL Definitions - Base types
 */

#ifndef __STM32F1XX_HAL_DEF_H
#define __STM32F1XX_HAL_DEF_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* GPIO Pin State */
typedef enum {
  GPIO_PIN_RESET = 0,
  GPIO_PIN_SET
} GPIO_PinState;

/* HAL Status */
typedef enum {
  HAL_OK = 0,
  HAL_ERROR,
  HAL_BUSY,
  HAL_TIMEOUT
} HAL_StatusTypeDef;

/* HAL Lock */
typedef enum {
  HAL_UNLOCKED = 0,
  HAL_LOCKED
} HAL_LockTypeDef;

/* NULL Pointer */
#ifndef NULL
#define NULL 0
#endif

/* Weak Definition */
#ifndef __weak
#define __weak __attribute__((weak))
#endif

#ifdef __cplusplus
}
#endif

#endif /* __STM32F1XX_HAL_DEF_H */