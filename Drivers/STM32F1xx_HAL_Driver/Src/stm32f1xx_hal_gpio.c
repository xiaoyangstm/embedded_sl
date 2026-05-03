/**
 * @file stm32f1xx_hal_gpio.c
 * @brief HAL GPIO Implementation
 */

#include "stm32f1xx_hal_gpio.h"

void HAL_GPIO_Init(GPIO_TypeDef *GPIOx, GPIO_InitTypeDef *GPIO_Init)
{
  uint32_t position;
  uint32_t currentmode;
  uint32_t currentpin;

  for (position = 0; position < 16; position++) {
    currentpin = (1 << position);

    if ((GPIO_Init->Pin & currentpin) == currentpin) {
      /* Mode configuration */
      currentmode = GPIO_Init->Mode;

      if (currentmode == GPIO_MODE_INPUT) {
        /* Input mode */
        if (GPIO_Init->Pull == GPIO_PULLUP) {
          GPIOx->ODR |= currentpin;
        } else {
          GPIOx->ODR &= ~currentpin;
        }
      }

      /* Configure GPIO */
      if (position < 8) {
        /* CRL register */
        GPIOx->CRL &= ~(0xF << (position * 4));
        GPIOx->CRL |= ((currentmode & 0xF) << (position * 4));
      } else {
        /* CRH register */
        GPIOx->CRH &= ~(0xF << ((position - 8) * 4));
        GPIOx->CRH |= ((currentmode & 0xF) << ((position - 8) * 4));
      }
    }
  }
}

void HAL_GPIO_DeInit(GPIO_TypeDef *GPIOx, uint32_t GPIO_Pin)
{
  uint32_t position;

  for (position = 0; position < 16; position++) {
    if ((GPIO_Pin & (1 << position)) == (1 << position)) {
      /* Configure as analog input */
      if (position < 8) {
        GPIOx->CRL &= ~(0xF << (position * 4));
        GPIOx->CRL |= (0x4 << (position * 4));
      } else {
        GPIOx->CRH &= ~(0xF << ((position - 8) * 4));
        GPIOx->CRH |= (0x4 << ((position - 8) * 4));
      }
    }
  }
}

GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
  return (GPIO_PinState)((GPIOx->IDR & GPIO_Pin) != 0);
}

void HAL_GPIO_WritePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState)
{
  if (PinState != GPIO_PIN_RESET) {
    GPIOx->BSRR = GPIO_Pin;
  } else {
    GPIOx->BRR = GPIO_Pin;
  }
}

void HAL_GPIO_TogglePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
  GPIOx->ODR ^= GPIO_Pin;
}