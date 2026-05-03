/**
 * @file stm32f1xx_hal_pwr.c
 * @brief HAL PWR Implementation
 */

#include "stm32f1xx_hal_pwr.h"

void HAL_PWR_EnableBkUpAccess(void)
{
  /* Enable backup domain access */
  RCC->APB1ENR |= (1 << 27);  /* BKPEN */
  RCC->APB1ENR |= (1 << 28);  /* PWREN */
  PWR->CR |= (1 << 8);        /* DBP */
}

void HAL_PWR_DisableBkUpAccess(void)
{
  /* Disable backup domain access */
  PWR->CR &= ~(1 << 8);
}