/**
 * @file stm32f1xx_hal.c
 * @brief HAL Main Implementation
 */

#include "stm32f1xx_hal.h"

static uint32_t uwTick = 0;

void HAL_Init(void)
{
  /* Enable SysTick */
  SysTick->LOAD = 72000 - 1;  /* 1ms at 72MHz */
  SysTick->VAL = 0;
  SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;

  /* Init MSP */
  HAL_MspInit();
}

__weak void HAL_MspInit(void)
{
}

void HAL_Delay(uint32_t Delay)
{
  uint32_t tickstart = HAL_GetTick();
  while ((HAL_GetTick() - tickstart) < Delay) {
  }
}

uint32_t HAL_GetTick(void)
{
  return uwTick;
}

void HAL_IncTick(void)
{
  uwTick++;
}