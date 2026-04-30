/**
 * @file main.c
 * @brief ESP8266 MQTT 上传假数据到 EMQX (测试版)
 * STM32F103C8T6 @ 72MHz (HAL库)
 */

#include "stm32f1xx_hal.h"
#include "esp8266.h"
#include "onenet.h"
#include "mqttkit.h"
#include <stdio.h>
#include <string.h>

/* I2C和UART句柄 */
I2C_HandleTypeDef hi2c1;
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* ESP8266接收缓冲区 */
unsigned char esp8266_buf[512];
unsigned short esp8266_cnt = 0;

/* BMM150数据 - 全局变量 */
int16_t bmm_x = 0;
int16_t bmm_y = 0;
int16_t bmm_z = 0;

/* 72MHz时钟配置 */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef osc = {0};
    RCC_ClkInitTypeDef clk = {0};

    osc.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    osc.HSEState = RCC_HSE_ON;
    osc.PLL.PLLState = RCC_PLL_ON;
    osc.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    osc.PLL.PLLMUL = RCC_PLL_MUL9;
    HAL_RCC_OscConfig(&osc);

    clk.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                    RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    clk.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    clk.AHBCLKDivider = RCC_SYSCLK_DIV1;
    clk.APB1CLKDivider = RCC_HCLK_DIV2;
    clk.APB2CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_2);
}

/* USART1初始化 (调试输出) - PA9 TX, PA10 RX */
void USART1_Init(void)
{
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitTypeDef gpio = {0};

    /* PA9 TX */
    gpio.Pin = GPIO_PIN_9;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &gpio);

    /* PA10 RX */
    gpio.Pin = GPIO_PIN_10;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &gpio);

    /* 使用HAL库初始化USART1 */
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart1);
}

/* USART2初始化 (ESP8266通信) */
void USART2_Init(void)
{
    __HAL_RCC_USART2_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitTypeDef gpio = {0};
    gpio.Pin = GPIO_PIN_2;  /* TX */
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &gpio);

    gpio.Pin = GPIO_PIN_3;  /* RX */
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &gpio);

    huart2.Instance = USART2;
    huart2.Init.BaudRate = 115200;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart2);

    HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
    __HAL_UART_ENABLE_IT(&huart2, UART_IT_RXNE);
}

/* USART2中断接收 (ESP8266) */
void USART2_IRQHandler(void)
{
    if(__HAL_UART_GET_FLAG(&huart2, UART_FLAG_RXNE) != RESET)
    {
        if(esp8266_cnt >= sizeof(esp8266_buf))
            esp8266_cnt = 0;

        esp8266_buf[esp8266_cnt++] = (uint8_t)(huart2.Instance->DR & 0xFF);
        __HAL_UART_CLEAR_FLAG(&huart2, UART_FLAG_RXNE);
    }
}

/* 简单串口发送函数 */
void UART1_SendString(const char *str)
{
    HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), 1000);
}

/* 全局调试打印函数 - 供其他模块调用 */
void DebugPrint(const char *str)
{
    UART1_SendString(str);
}

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    /* LED PC13 */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    GPIO_InitTypeDef led = {0};
    led.Pin = GPIO_PIN_13;
    led.Mode = GPIO_MODE_OUTPUT_PP;
    led.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &led);

    /* 初始化USART1 */
    USART1_Init();

    /* 立即发送测试数据 - 不依赖其他初始化 */
    UART1_SendString("USART1 Test OK!\r\n");
    HAL_Delay(100);
    UART1_SendString("System Init...\r\n");
    HAL_Delay(100);

    /* 初始化USART2 */
    USART2_Init();
    UART1_SendString("USART2 Init OK\r\n");

    UART1_SendString("\r\n=================================\r\n");
    UART1_SendString("STM32F103C8T6 @ 72MHz\r\n");
    UART1_SendString("ESP8266 MQTT Test\r\n");
    UART1_SendString("=================================\r\n\r\n");

    /* 初始化ESP8266并连接MQTT */
    UART1_SendString("Initializing ESP8266...\r\n");
    ESP8266_Init();

    UART1_SendString("\r\nConnecting to MQTT...\r\n");
    if (OneNet_DevLink() != 0) {
        UART1_SendString("MQTT Connect FAILED!\r\n");
        while (1) {
            HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
            HAL_Delay(100);
        }
    }

    /* 订阅主题 */
    OneNET_Subscribe();

    UART1_SendString("\r\nStart sending fake data...\r\n\r\n");

    uint32_t count = 0;
    char msg_buf[100];

    /* Simple pseudo-random seed */
    uint32_t rand_seed = 12345;

    while (1) {
        /* Simple pseudo-random number generator (LCG) */
        rand_seed = (rand_seed * 1103515245 + 12345) & 0x7FFFFFFF;

        /* Random-like values for x, y, z */
        bmm_x = (int16_t)((rand_seed % 400) - 200);      /* -200 to 200 */
        rand_seed = (rand_seed * 1103515245 + 12345) & 0x7FFFFFFF;
        bmm_y = (int16_t)((rand_seed % 400) - 200);      /* -200 to 200 */
        rand_seed = (rand_seed * 1103515245 + 12345) & 0x7FFFFFFF;
        bmm_z = (int16_t)((rand_seed % 600) - 300);      /* -300 to 300 */

        sprintf(msg_buf, "[%lu] X:%d Y:%d Z:%d\r\n", count, bmm_x, bmm_y, bmm_z);
        UART1_SendString(msg_buf);

        /* 发送到MQTT */
        OneNet_SendData();

        /* LED闪烁 */
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        count++;

        /* 检查接收数据 */
        unsigned char *dataPtr = ESP8266_GetIPD(50);
        if (dataPtr != NULL) {
            OneNet_RevPro(dataPtr);
        }

        /* 定时发送心跳 */
        if (count % 30 == 0) {
            MQTT_Ping();
        }

        HAL_Delay(1000);
    }
}