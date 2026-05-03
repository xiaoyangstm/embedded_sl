/**
 * @file main.c
 * @brief BMM150 示例程序 - STM32F103C8T6
 */

#include "main.h"
#include "bmm150.h"
#include <stdio.h>

/* 私有变量 */
I2C_HandleTypeDef hi2c1;
static bmm150_dev_t bmm150_dev;
static bmm150_data_t mag_data;

/* 私有函数声明 */
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void BMM150_Print_Data(bmm150_data_t *data);

void SystemClock_Config(void)
{
  /* Enable HSE */
  RCC->CR |= 0x00010000;  /* HSEON */
  while ((RCC->CR & 0x00020000) == 0);  /* Wait for HSERDY */

  /* Configure PLL for 72MHz */
  RCC->CFGR |= 0x001D0400;  /* PLL configuration */

  /* Enable PLL */
  RCC->CR |= 0x01000000;  /* PLLON */
  while ((RCC->CR & 0x02000000) == 0);  /* Wait for PLLRDY */

  /* Select PLL as system clock */
  RCC->CFGR |= 0x00000002;
  while ((RCC->CFGR & 0x00000008) != 0x08);  /* Wait for switch */

  /* Enable APB clocks */
  RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN | RCC_APB2ENR_AFIOEN;
}

int main(void)
{
    /* MCU初始化 */
    HAL_Init();
    SystemClock_Config();

    /* 外设初始化 */
    MX_GPIO_Init();
    MX_I2C1_Init();

    /* 初始化BMM150 */
    bmm150_status_t status = bmm150_init(&bmm150_dev, BMM150_DEFAULT_I2C_ADDRESS);

    if (status != BMM150_OK) {
        /* 初始化失败处理 */
        Error_Handler();
    }

    /* 配置BMM150 - 高精度模式 */
    bmm150_set_xy_rep(&bmm150_dev, BMM150_REPXY_HIGHACCURACY);
    bmm150_set_z_rep(&bmm150_dev, BMM150_REPZ_HIGHACCURACY);
    bmm150_set_data_rate(&bmm150_dev, BMM150_DATA_RATE_10HZ);
    bmm150_set_op_mode(&bmm150_dev, BMM150_NORMAL_MODE);

    /* 主循环 */
    while (1)
    {
        /* 读取磁力数据 */
        status = bmm150_get_mag_data(&bmm150_dev, &mag_data);

        if (status == BMM150_OK) {
            /* 打印数据 */
            BMM150_Print_Data(&mag_data);
        }

        /* 延时100ms */
        HAL_Delay(100);
    }
}

static void MX_I2C1_Init(void)
{
    hi2c1.Instance = I2C1;
    hi2c1.Init.ClockSpeed = 100000;     /* 100kHz */
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    if (HAL_I2C_Init(&hi2c1) != HAL_OK) {
        Error_Handler();
    }
}

static void MX_GPIO_Init(void)
{
    /* Enable GPIO clocks */
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN;
}

static void BMM150_Print_Data(bmm150_data_t *data)
{
    /* 注意: 实际使用时需要配置USART输出 */
    /* 这里只是演示数据格式 */

    /* 简单的LED闪烁表示数据更新 */
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
}

void Error_Handler(void)
{
    /* Disable interrupts using inline assembly */
    __asm volatile ("cpsid i");
    while (1) {
    }
}

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
}
#endif /* USE_FULL_ASSERT */