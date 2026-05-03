/**
 * @file stm32f1xx_hal_tim.c
 * @brief HAL TIM Implementation
 */

#include "stm32f1xx_hal_tim.h"

HAL_StatusTypeDef HAL_TIM_Base_Init(TIM_HandleTypeDef *htim)
{
  if (htim == NULL) {
    return HAL_ERROR;
  }

  /* Set prescaler */
  htim->Instance->PSC = htim->Init.Prescaler;

  /* Set period */
  htim->Instance->ARR = htim->Init.Period;

  /* Set counter mode */
  if (htim->Instance == TIM1) {
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
  } else if (htim->Instance == TIM2) {
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
  } else if (htim->Instance == TIM3) {
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
  } else if (htim->Instance == TIM4) {
    RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
  }

  HAL_TIM_Base_MspInit(htim);

  return HAL_OK;
}

HAL_StatusTypeDef HAL_TIM_Base_DeInit(TIM_HandleTypeDef *htim)
{
  if (htim == NULL) {
    return HAL_ERROR;
  }

  /* Disable TIM */
  htim->Instance->CR1 &= ~1;

  HAL_TIM_Base_MspDeInit(htim);

  return HAL_OK;
}

__weak void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
}

__weak void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef *htim)
{
}

HAL_StatusTypeDef HAL_TIM_Base_Start(TIM_HandleTypeDef *htim)
{
  /* Enable counter */
  htim->Instance->CR1 |= 1;

  return HAL_OK;
}

HAL_StatusTypeDef HAL_TIM_Base_Stop(TIM_HandleTypeDef *htim)
{
  /* Disable counter */
  htim->Instance->CR1 &= ~1;

  return HAL_OK;
}