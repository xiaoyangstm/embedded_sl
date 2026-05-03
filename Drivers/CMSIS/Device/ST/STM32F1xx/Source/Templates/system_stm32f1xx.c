/**
 * @file system_stm32f1xx.c
 * @brief STM32F1xx System Clock Configuration
 */

#include "stm32f1xx.h"

uint32_t SystemCoreClock = 72000000;  /* 72 MHz default */

void SystemInit(void)
{
    /* Reset RCC */
    RCC->CR |= 0x00000001;  /* Enable HSI */
    RCC->CFGR = 0x00000000; /* Reset CFGR */
    RCC->CR &= ~(0x00001000); /* Disable HSE */
    RCC->CIR = 0x00000000;  /* Disable all interrupts */
}