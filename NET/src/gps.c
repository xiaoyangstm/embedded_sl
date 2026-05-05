/**
 * @file gps.c
 * @brief GPS模块驱动实现 - 中科微电子ATGM336H
 *        使用全局缓冲数组存储解析结果
 */

#include "gps.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* GPS接收缓冲区 */
#define GPS_BUF_SIZE    512
static char gps_buf[GPS_BUF_SIZE];
static uint16_t gps_idx = 0;

/* 全局缓冲数组 - 存储经纬度字符串 */
static char g_lat_str[20] = {0};
static char g_lon_str[20] = {0};
static char g_ns = 'N';
static char g_ew = 'E';
static uint8_t g_rmc_valid = 0;

/* GPS输出数据 - 使用volatile确保数据一致性 */
static gps_data_t gps_data = {0};
static uint8_t gps_new_data = 0;
static uint8_t gps_data_ready = 0;  /* 数据已就绪标志 */

/* USART3句柄 */
extern UART_HandleTypeDef huart3;

/* 调试输出函数 */
extern void UART1_SendString(const char *str);

/**
 * @brief 初始化USART3用于GPS通信
 */
void GPS_Init(void)
{
    __HAL_RCC_USART3_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef gpio = {0};

    gpio.Pin = GPIO_PIN_10;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &gpio);

    gpio.Pin = GPIO_PIN_11;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOB, &gpio);

    huart3.Instance = USART3;
    huart3.Init.BaudRate = 9600;
    huart3.Init.WordLength = UART_WORDLENGTH_8B;
    huart3.Init.StopBits = UART_STOPBITS_1;
    huart3.Init.Parity = UART_PARITY_NONE;
    huart3.Init.Mode = UART_MODE_TX_RX;
    huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart3.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart3);

    HAL_NVIC_SetPriority(USART3_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(USART3_IRQn);
    __HAL_UART_ENABLE_IT(&huart3, UART_IT_RXNE);

    memset(&gps_data, 0, sizeof(gps_data));
    memset(g_lat_str, 0, sizeof(g_lat_str));
    memset(g_lon_str, 0, sizeof(g_lon_str));
    g_ns = 'N';
    g_ew = 'E';
    g_rmc_valid = 0;
    gps_idx = 0;
    gps_new_data = 0;
}

/**
 * @brief 将NMEA格式的坐标转换为十进制度数（整数版本）
 * NMEA格式: ddmm.mmmm 或 dddmm.mmmm
 * 返回: 整数形式的十进制度数 * 1000000 (微度)
 */
static int32_t Convert_to_degrees_int(char* data)
{
    if (data == NULL || strlen(data) == 0) return 0;

    /* 手动解析浮点数 */
    int32_t raw = 0;
    int i = 0;
    int32_t sign = 1;

    /* 跳过前导空格 */
    while (data[i] == ' ') i++;

    /* 处理符号 */
    if (data[i] == '-') {
        sign = -1;
        i++;
    } else if (data[i] == '+') {
        i++;
    }

    /* 解析整数部分 */
    while (data[i] >= '0' && data[i] <= '9') {
        raw = raw * 10 + (data[i] - '0');
        i++;
    }

    /* 解析小数部分 (最多4位) */
    int32_t fraction = 0;
    int frac_digits = 0;
    if (data[i] == '.') {
        i++;
        while (data[i] >= '0' && data[i] <= '9' && frac_digits < 4) {
            fraction = fraction * 10 + (data[i] - '0');
            frac_digits++;
            i++;
        }
        /* 补齐4位小数 */
        while (frac_digits < 4) {
            fraction *= 10;
            frac_digits++;
        }
    }

    /* 合成完整数值: raw * 10000 + fraction */
    int32_t full_value = raw * 10000 + fraction;
    full_value *= sign;

    /* 提取度数部分 */
    int32_t degrees = full_value / 1000000;  /* 除以1000000得到度 */

    /* 提取分钟部分 */
    int32_t minutes_full = full_value - degrees * 1000000;
    int32_t minutes = minutes_full / 10000;  /* 得到分钟 */

    /* 分钟的小数部分 */
    int32_t minutes_frac = minutes_full % 10000;

    /* 转换为十进制度数 (微度) */
    /* degrees + minutes/60 + minutes_frac/(60*10000) */
    /* = degrees * 1000000 + minutes * 16666 + minutes_frac * 1.6666 */
    /* 简化计算: minutes/60 = minutes * 1000000 / 60000000 */
    int32_t result = degrees * 1000000;
    result += minutes * 16666;  /* minutes / 60 * 1000000 ≈ minutes * 16666 */
    result += minutes_frac * 1666 / 10000;  /* minutes_frac / 60 / 10000 * 1000000 */

    return result;
}

/**
 * @brief 解析$GNRMC语句 - 存入全局缓冲数组并更新输出
 */
static void Parse_GNRMC(char *buffer)
{
    char *p, *comma;
    int fieldIndex = 0;
    char fieldBuf[32];
    char status = 'V';

    p = strstr(buffer, "$GNRMC");
    if (p == NULL) p = strstr(buffer, "$GPRMC");
    if (p == NULL) return;

    comma = strchr(p, ',');
    if (comma == NULL) return;
    p = comma + 1;

    /* 临时变量 */
    char latTmp[20] = {0};
    char lonTmp[20] = {0};
    char nsTmp = 'N', ewTmp = 'E';

    while ((comma = strchr(p, ',')) != NULL)
    {
        int len = comma - p;
        if (len > 0 && len < 32)
        {
            memcpy(fieldBuf, p, len);
            fieldBuf[len] = '\0';

            switch (fieldIndex)
            {
                case 1: status = fieldBuf[0]; break;
                case 2: strncpy(latTmp, fieldBuf, sizeof(latTmp)-1); break;
                case 3: nsTmp = fieldBuf[0]; break;
                case 4: strncpy(lonTmp, fieldBuf, sizeof(lonTmp)-1); break;
                case 5: ewTmp = fieldBuf[0]; break;
            }
        }
        p = comma + 1;
        fieldIndex++;
        if (fieldIndex > 5) break;
    }

    /* 存入全局缓冲数组 */
    if (status == 'A' && strlen(latTmp) > 0 && strlen(lonTmp) > 0)
    {
        strncpy(g_lat_str, latTmp, sizeof(g_lat_str)-1);
        g_lat_str[sizeof(g_lat_str)-1] = '\0';
        strncpy(g_lon_str, lonTmp, sizeof(g_lon_str)-1);
        g_lon_str[sizeof(g_lon_str)-1] = '\0';
        g_ns = nsTmp;
        g_ew = ewTmp;
        g_rmc_valid = 1;

        /* 立即更新输出数据 - 使用整数 */
        int32_t lat = Convert_to_degrees_int(g_lat_str);
        int32_t lon = Convert_to_degrees_int(g_lon_str);

        if (g_ns == 'S') lat = -lat;
        if (g_ew == 'W') lon = -lon;

        gps_data.latitude = lat;
        gps_data.longitude = lon;
        gps_data.valid = 1;
        gps_data_ready = 1;
        gps_new_data = 1;
    }
    /* 只有在没有有效数据时才更新valid标志 */
    else if (gps_data_ready == 0)
    {
        g_rmc_valid = 0;
        gps_data.valid = 0;
    }
}

/**
 * @brief 解析$GNGGA语句 - 更新卫星数量
 */
static void Parse_GNGGA(char *buffer)
{
    char *p, *comma;
    int fieldIndex = 0;
    char fieldBuf[32];

    p = strstr(buffer, "$GNGGA");
    if (p == NULL) p = strstr(buffer, "$GPGGA");
    if (p == NULL) return;

    comma = strchr(p, ',');
    if (comma == NULL) return;
    p = comma + 1;

    while ((comma = strchr(p, ',')) != NULL)
    {
        int len = comma - p;
        if (len > 0 && len < 32)
        {
            memcpy(fieldBuf, p, len);
            fieldBuf[len] = '\0';

            if (fieldIndex == 6)
            {
                gps_data.sat_count = (uint8_t)atoi(fieldBuf);
                gps_new_data = 1;
                break;
            }
        }
        p = comma + 1;
        fieldIndex++;
    }
}

/**
 * @brief USART3中断服务函数
 */
void USART3_IRQHandler(void)
{
    if(__HAL_UART_GET_FLAG(&huart3, UART_FLAG_RXNE) != RESET)
    {
        char c = (char)(huart3.Instance->DR & 0xFF);

        if (c == '$')
        {
            gps_idx = 0;
            gps_buf[gps_idx++] = c;
        }
        else if (c == '\n' || c == '\r')
        {
            if (gps_idx > 6)
            {
                gps_buf[gps_idx] = '\0';

                if (strstr(gps_buf, "$GNRMC") != NULL || strstr(gps_buf, "$GPRMC") != NULL)
                {
                    Parse_GNRMC(gps_buf);
                }
                else if (strstr(gps_buf, "$GNGGA") != NULL || strstr(gps_buf, "$GPGGA") != NULL)
                {
                    Parse_GNGGA(gps_buf);
                }
            }
            gps_idx = 0;
        }
        else if (gps_idx < GPS_BUF_SIZE - 1)
        {
            gps_buf[gps_idx++] = c;
        }

        __HAL_UART_CLEAR_FLAG(&huart3, UART_FLAG_RXNE);
    }
}

void GPS_Parse(void)
{
    /* 数据已在USART中断中实时解析 */
}

gps_data_t* GPS_GetData(void)
{
    return &gps_data;
}

uint8_t GPS_HasNewData(void)
{
    return gps_new_data;
}

uint8_t GPS_IsDataReady(void)
{
    return gps_data_ready;
}

void GPS_ClearNewData(void)
{
    gps_new_data = 0;
}
