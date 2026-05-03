/**
 * @file stm32f1xx_hal_flash.c
 * @brief HAL FLASH Implementation
 */

#include "stm32f1xx_hal_flash.h"

void HAL_FLASH_Unlock(void)
{
  /* Unlock flash */
  FLASH->KEYR = 0x45670123;
  FLASH->KEYR = 0xCDEF89AB;
}

void HAL_FLASH_Lock(void)
{
  /* Lock flash */
}