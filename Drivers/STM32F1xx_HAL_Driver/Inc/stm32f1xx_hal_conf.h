/**
 * @file stm32f1xx_hal_conf.h
 * @brief HAL Configuration File
 */

#ifndef __STM32F1XX_HAL_CONF_H
#define __STM32F1XX_HAL_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f103xb.h"

/* Module Enable */
#define HAL_MODULE_ENABLED
#define HAL_GPIO_MODULE_ENABLED
#define HAL_I2C_MODULE_ENABLED
#define HAL_RCC_MODULE_ENABLED
#define HAL_FLASH_MODULE_ENABLED
#define HAL_PWR_MODULE_ENABLED
#define HAL_CORTEX_MODULE_ENABLED
#define HAL_TIM_MODULE_ENABLED

/* HSE Value */
#define HSE_VALUE    8000000U

/* SysTick Value */
#define TICK_INT_PRIORITY  0x0F

/* Exported Types */
typedef enum {
  HAL_OK = 0,
  HAL_ERROR,
  HAL_BUSY,
  HAL_TIMEOUT
} HAL_StatusTypeDef;

typedef enum {
  HAL_UNLOCKED = 0,
  HAL_LOCKED
} HAL_LockTypeDef;

/* Include Headers */
#ifdef HAL_MODULE_ENABLED
  #include "stm32f1xx_hal.h"
#endif

#ifdef HAL_GPIO_MODULE_ENABLED
  #include "stm32f1xx_hal_gpio.h"
#endif

#ifdef HAL_I2C_MODULE_ENABLED
  #include "stm32f1xx_hal_i2c.h"
#endif

#ifdef HAL_RCC_MODULE_ENABLED
  #include "stm32f1xx_hal_rcc.h"
#endif

#ifdef HAL_FLASH_MODULE_ENABLED
  #include "stm32f1xx_hal_flash.h"
#endif

#ifdef HAL_PWR_MODULE_ENABLED
  #include "stm32f1xx_hal_pwr.h"
#endif

#ifdef HAL_CORTEX_MODULE_ENABLED
  #include "stm32f1xx_hal_cortex.h"
#endif

#ifdef HAL_TIM_MODULE_ENABLED
  #include "stm32f1xx_hal_tim.h"
#endif

#ifdef __cplusplus
}
#endif

#endif /* __STM32F1XX_HAL_CONF_H */