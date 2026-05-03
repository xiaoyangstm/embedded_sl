/**
 * @file main.c
 * @brief BMM150 磁力计数据通过 ESP8266 MQTT 上传到 OneNET
 * STM32F103C8T6 @ 72MHz (HAL库)
 */

#include "stm32f1xx_hal.h"
#include "esp8266.h"
#include "onenet.h"
#include "mqttkit.h"
#include "bmm150.h"
#include "bmm150_platform.h"
#include <stdio.h>
#include <string.h>

/* I2C和UART句柄 */
I2C_HandleTypeDef hi2c1;
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* ESP8266接收缓冲区 */
unsigned char esp8266_buf[512];
unsigned short esp8266_cnt = 0;

/* BMM150设备句柄和数据 */
bmm150_dev_t bmm150_dev;
bmm150_data_t bmm150_data;

/* BMM150全局数据 - 供onenet.c通过extern引用 */
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

/* I2C1初始化 (BMM150通信) - PB6 SCL, PB7 SDA */
void I2C1_Init(void)
{
    __HAL_RCC_I2C1_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef gpio = {0};

    /* PB6 SCL, PB7 SDA */
    gpio.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    gpio.Mode = GPIO_MODE_AF_OD;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    gpio.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOB, &gpio);

    /* I2C1配置 */
    hi2c1.Instance = I2C1;
    hi2c1.Init.ClockSpeed = 100000;  /* 100kHz */
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    HAL_I2C_Init(&hi2c1);
}

/* I2C总线扫描 - 查找连接的设备 */
void I2C_Scan(void)
{
    char buf[50];
    UART1_SendString("Scanning I2C bus...\r\n");

    for (uint8_t addr = 0; addr < 128; addr++) {
        if (HAL_I2C_IsDeviceReady(&hi2c1, (uint16_t)(addr << 1), 1, 10) == HAL_OK) {
            sprintf(buf, "Found device at 0x%02X (7-bit: 0x%02X)\r\n", addr << 1, addr);
            UART1_SendString(buf);
        }
    }
    UART1_SendString("Scan complete.\r\n");
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

    /* 初始化I2C1 */
    I2C1_Init();
    UART1_SendString("I2C1 Init OK\r\n");

    /* 扫描I2C总线，查找BMM150 */
    I2C_Scan();

    /* 初始化BMM150 */
    char msg_buf[100];
    UART1_SendString("Initializing BMM150...\r\n");

    /* 步骤1: 唤醒BMM150 - 写入0x01到电源控制寄存器0x4B */
    uint8_t pwr_cmd = 0x01;
    HAL_I2C_Mem_Write(&hi2c1, (BMM150_DEFAULT_I2C_ADDRESS << 1), 0x4B, I2C_MEMADD_SIZE_8BIT, &pwr_cmd, 1, 100);
    HAL_Delay(10);  /* 等待唤醒 */

    /* 步骤2: 再次读取Chip ID */
    uint8_t chip_id = 0;
    HAL_I2C_Mem_Read(&hi2c1, (BMM150_DEFAULT_I2C_ADDRESS << 1), 0x40, I2C_MEMADD_SIZE_8BIT, &chip_id, 1, 100);
    sprintf(msg_buf, "Chip ID after wake: 0x%02X (expected: 0x32)\r\n", chip_id);
    UART1_SendString(msg_buf);

    bmm150_status_t status = bmm150_init(&bmm150_dev, BMM150_DEFAULT_I2C_ADDRESS);
    if (status != BMM150_OK) {
        sprintf(msg_buf, "BMM150 Init FAILED! Error: %d\r\n", status);
        UART1_SendString(msg_buf);
        while (1) {
            HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
            HAL_Delay(100);
        }
    }
    UART1_SendString("BMM150 Init OK\r\n");

    UART1_SendString("\r\n=================================\r\n");
    UART1_SendString("STM32F103C8T6 @ 72MHz\r\n");
    UART1_SendString("BMM150 + ESP8266 MQTT\r\n");
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

    UART1_SendString("\r\nStart reading BMM150 sensor data...\r\n\r\n");

    uint32_t count = 0;

    while (1) {
        /* 从BMM150读取真实传感器数据 */
        status = bmm150_get_mag_data(&bmm150_dev, &bmm150_data);
        if (status == BMM150_OK) {
            bmm_x = (int16_t)bmm150_data.x;
            bmm_y = (int16_t)bmm150_data.y;
            bmm_z = (int16_t)bmm150_data.z;

            sprintf(msg_buf, "[%lu] X:%d Y:%d Z:%d (%.1f %.1f %.1f uT)\r\n",
                    count, bmm_x, bmm_y, bmm_z,
                    bmm150_data.x_uT, bmm150_data.y_uT, bmm150_data.z_uT);
            UART1_SendString(msg_buf);
        } else {
            sprintf(msg_buf, "[%lu] BMM150 read error: %d\r\n", count, status);
            UART1_SendString(msg_buf);
        }

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