/**
 * @file stm32f1xx_hal_msp.c
 * @brief HAL MSP Initialization
 */

#include "main.h"

I2C_HandleTypeDef hi2c1;

void HAL_MspInit(void)
{
  /* Enable AFIO clock */
  RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
}

void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  if (hi2c->Instance == I2C1) {
    /* Enable GPIOB clock */
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;

    /* Configure I2C pins: PB6 (SCL), PB7 (SDA) */
    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* Enable I2C1 clock */
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
  }
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef *hi2c)
{
  if (hi2c->Instance == I2C1) {
    /* Disable I2C1 clock */
    RCC->APB1ENR &= ~RCC_APB1ENR_I2C1EN;

    /* Deinitialize I2C pins */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6 | GPIO_PIN_7);
  }
}