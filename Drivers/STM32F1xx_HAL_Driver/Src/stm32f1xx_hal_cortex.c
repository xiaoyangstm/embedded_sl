/**
 * @file stm32f1xx_hal_cortex.c
 * @brief HAL Cortex Implementation
 */

#include "stm32f1xx_hal_cortex.h"

void HAL_NVIC_SetPriority(uint8_t IRQn, uint32_t PriorityGroup, uint32_t PreemptPriority, uint32_t SubPriority)
{
  NVIC->IP[IRQn] = ((PreemptPriority << 4) & 0xF0);
}

void HAL_NVIC_EnableIRQ(uint8_t IRQn)
{
  NVIC->ISER[0] = (1 << IRQn);
}

void HAL_NVIC_DisableIRQ(uint8_t IRQn)
{
  NVIC->ICER[0] = (1 << IRQn);
}

void HAL_SYSTICK_Config(uint32_t TicksNumb)
{
  SysTick->LOAD = TicksNumb - 1;
  SysTick->VAL = 0;
  SysTick->CTRL = 0x07;  /* Enable SysTick */
}