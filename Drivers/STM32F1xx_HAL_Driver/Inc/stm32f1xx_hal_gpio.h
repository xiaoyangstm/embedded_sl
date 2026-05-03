/**
 * @file stm32f1xx_hal_gpio.h
 * @brief HAL GPIO Header
 */

#ifndef __STM32F1XX_HAL_GPIO_H
#define __STM32F1XX_HAL_GPIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal_def.h"

/* GPIO Init Structure */
typedef struct {
  uint32_t Pin;
  uint32_t Mode;
  uint32_t Pull;
  uint32_t Speed;
} GPIO_InitTypeDef;

/* GPIO Pin Definitions */
#define GPIO_PIN_0                 0x0001
#define GPIO_PIN_1                 0x0002
#define GPIO_PIN_2                 0x0004
#define GPIO_PIN_3                 0x0008
#define GPIO_PIN_4                 0x0010
#define GPIO_PIN_5                 0x0020
#define GPIO_PIN_6                 0x0040
#define GPIO_PIN_7                 0x0080
#define GPIO_PIN_8                 0x0100
#define GPIO_PIN_9                 0x0200
#define GPIO_PIN_10                0x0400
#define GPIO_PIN_11                0x0800
#define GPIO_PIN_12                0x1000
#define GPIO_PIN_13                0x2000
#define GPIO_PIN_14                0x4000
#define GPIO_PIN_15                0x8000
#define GPIO_PIN_ALL               0xFFFF

/* GPIO Mode */
#define GPIO_MODE_INPUT            0x00
#define GPIO_MODE_OUTPUT_PP        0x01
#define GPIO_MODE_OUTPUT_OD        0x02
#define GPIO_MODE_AF_PP            0x03
#define GPIO_MODE_AF_OD            0x04
#define GPIO_MODE_ANALOG           0x05
#define GPIO_MODE_IT_RISING        0x10
#define GPIO_MODE_IT_FALLING       0x20

/* GPIO Speed */
#define GPIO_SPEED_FREQ_LOW        0x00
#define GPIO_SPEED_FREQ_MEDIUM     0x01
#define GPIO_SPEED_FREQ_HIGH       0x02

/* GPIO Pull */
#define GPIO_NOPULL                0x00
#define GPIO_PULLUP                0x01
#define GPIO_PULLDOWN              0x02

/* Function Declarations */
void HAL_GPIO_Init(GPIO_TypeDef *GPIOx, GPIO_InitTypeDef *GPIO_Init);
void HAL_GPIO_DeInit(GPIO_TypeDef *GPIOx, uint32_t GPIO_Pin);
GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
void HAL_GPIO_WritePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState);
void HAL_GPIO_TogglePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);

#ifdef __cplusplus
}
#endif

#endif /* __STM32F1XX_HAL_GPIO_H */