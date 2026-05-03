/**
 * @file stm32f1xx_hal_tim.h
 * @brief HAL TIM Header
 */

#ifndef __STM32F1XX_HAL_TIM_H
#define __STM32F1XX_HAL_TIM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal_def.h"

/* TIM Init Structure */
typedef struct {
  uint32_t Prescaler;
  uint32_t CounterMode;
  uint32_t Period;
  uint32_t ClockDivision;
  uint32_t AutoReloadPreload;
} TIM_Base_InitTypeDef;

/* TIM Handle Structure */
typedef struct {
  TIM_TypeDef *Instance;
  TIM_Base_InitTypeDef Init;
  HAL_LockTypeDef Lock;
  uint32_t State;
} TIM_HandleTypeDef;

/* TIM Counter Mode */
#define TIM_COUNTERMODE_UP          0x00000000
#define TIM_COUNTERMODE_DOWN        0x00000010
#define TIM_COUNTERMODE_CENTERALIGNED1 0x00000001
#define TIM_COUNTERMODE_CENTERALIGNED2 0x00000002
#define TIM_COUNTERMODE_CENTERALIGNED3 0x00000003

/* TIM Clock Division */
#define TIM_CLOCKDIVISION_DIV1      0x00000000
#define TIM_CLOCKDIVISION_DIV2      0x00000010
#define TIM_CLOCKDIVISION_DIV4      0x00000020

/* TIM AutoReload Preload */
#define TIM_AUTORELOAD_PRELOAD_DISABLE 0x00000000
#define TIM_AUTORELOAD_PRELOAD_ENABLE  0x00000080

/* Function Declarations */
HAL_StatusTypeDef HAL_TIM_Base_Init(TIM_HandleTypeDef *htim);
HAL_StatusTypeDef HAL_TIM_Base_DeInit(TIM_HandleTypeDef *htim);
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim);
void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef *htim);
HAL_StatusTypeDef HAL_TIM_Base_Start(TIM_HandleTypeDef *htim);
HAL_StatusTypeDef HAL_TIM_Base_Stop(TIM_HandleTypeDef *htim);

#ifdef __cplusplus
}
#endif

#endif /* __STM32F1XX_HAL_TIM_H */