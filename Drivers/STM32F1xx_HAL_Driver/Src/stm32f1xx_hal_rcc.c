/**
 * @file stm32f1xx_hal_rcc.c
 * @brief HAL RCC Implementation
 */

#include "stm32f1xx_hal_rcc.h"

extern uint32_t SystemCoreClock;

void HAL_RCC_ClockConfig(void)
{
  /* Enable HSE */
  RCC->CR |= 0x00010000;  /* HSEON */
  while ((RCC->CR & 0x00020000) == 0);  /* Wait for HSERDY */

  /* Configure PLL */
  RCC->CFGR |= 0x001D0400;  /* PLL configuration for 72MHz */

  /* Enable PLL */
  RCC->CR |= 0x01000000;  /* PLLON */
  while ((RCC->CR & 0x02000000) == 0);  /* Wait for PLLRDY */

  /* Select PLL as system clock */
  RCC->CFGR |= 0x00000002;
  while ((RCC->CFGR & 0x00000008) != 0x08);  /* Wait for switch */

  /* Enable APB clocks */
  RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN | RCC_APB2ENR_AFIOEN;

  SystemCoreClock = 72000000;
}

void HAL_RCC_OscConfig(void)
{
  /* Default implementation */
}

uint32_t HAL_RCC_GetPCLK1Freq(void)
{
  return SystemCoreClock / 2;  /* PCLK1 is half of SYSCLK */
}

uint32_t HAL_RCC_GetPCLK2Freq(void)
{
  return SystemCoreClock;  /* PCLK2 equals SYSCLK */
}

uint32_t HAL_RCC_GetSysClockFreq(void)
{
  return SystemCoreClock;
}