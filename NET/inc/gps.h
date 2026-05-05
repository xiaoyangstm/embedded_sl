/**
 * @file gps.h
 * @brief GPS模块驱动 - 中科微电子ATGM336H
 *        解析NMEA协议，提取经纬度信息
 */

#ifndef __GPS_H
#define __GPS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

/* GPS数据结构 */
typedef struct {
    int32_t latitude;        /* 纬度 (微度, 实际值 = latitude / 1000000) */
    int32_t longitude;       /* 经度 (微度, 实际值 = longitude / 1000000) */
    uint8_t valid;           /* 数据有效标志: 1=有效, 0=无效 */
    uint8_t sat_count;      /* 卫星数量 */
} gps_data_t;

/* GPS初始化 */
void GPS_Init(void);

/* 解析GPS数据 - 在主循环中调用 */
void GPS_Parse(void);

/* 获取GPS数据 */
gps_data_t* GPS_GetData(void);

/* 检查是否有新数据 */
uint8_t GPS_HasNewData(void);

/* 检查数据是否已就绪 */
uint8_t GPS_IsDataReady(void);

/* 清除新数据标志 */
void GPS_ClearNewData(void);

#ifdef __cplusplus
}
#endif

#endif /* __GPS_H */
