/**
 * @file bmm150.h
 * @brief BMM150 三轴地磁传感器驱动
 * @note 适用于 STM32F103C8T6
 */

#ifndef __BMM150_H
#define __BMM150_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* BMM150 I2C 地址 */
#define BMM150_DEFAULT_I2C_ADDRESS     0x10    /* 默认I2C地址 */
#define BMM150_I2C_ADDRESS_2           0x11    /* 替代I2C地址 */

/* BMM150 寄存器地址 */
#define BMM150_CHIP_ID_REG             0x40
#define BMM150_DATA_X_LSB              0x42
#define BMM150_DATA_X_MSB              0x43
#define BMM150_DATA_Y_LSB              0x44
#define BMM150_DATA_Y_MSB              0x45
#define BMM150_DATA_Z_LSB              0x46
#define BMM150_DATA_Z_MSB              0x47
#define BMM150_DATA_READY_STATUS       0x48
#define BMM150_INTERRUPT_STATUS         0x4A
#define BMM150_POWER_CONTROL_REG        0x4B
#define BMM150_OP_MODE_REG              0x4C
#define BMM150_INT_CONFIG_REG           0x4D
#define BMM150_AXES_ENABLE_REG          0x4E
#define BMM150_LOW_THRES_INT_REG        0x4F
#define BMM150_HIGH_THRES_INT_REG       0x50
#define BMM150_REP_XY_REG               0x51
#define BMM150_REP_Z_REG                0x52
#define BMM150_TRIM_XLSB_REG            0x5D
#define BMM150_TRIM_XMSB_REG            0x5E
#define BMM150_TRIM_YLSB_REG            0x5F
#define BMM150_TRIM_YMSB_REG            0x60
#define BMM150_TRIM_ZLSB_REG            0x61
#define BMM150_TRIM_ZMSB_REG            0x62
#define BMM150_TRIM_DATA_REG            0x63
#define BMM150_BIST_CTRL_REG            0x64
#define BMM150_BIST_STATUS_REG          0x65

/* 芯片ID */
#define BMM150_CHIP_ID                  0x32

/* 电源控制位 */
#define BMM150_POWER_CNTRL_SOFT_RESET   0x80
#define BMM150_POWER_CNTRL_SRST_DONE     0x01
#define BMM150_POWER_CNTRL_MSK          0x01

/* 操作模式 */
#define BMM150_NORMAL_MODE              0x00
#define BMM150_FORCED_MODE             0x01
#define BMM150_SLEEP_MODE              0x03
#define BMM150_SUSPEND_MODE            0x04

/* 数据速率 (ODR) */
#define BMM150_DATA_RATE_10HZ          0x00
#define BMM150_DATA_RATE_02HZ          0x01
#define BMM150_DATA_RATE_06HZ          0x02
#define BMM150_DATA_RATE_08HZ          0x03
#define BMM150_DATA_RATE_15HZ          0x04
#define BMM150_DATA_RATE_20HZ          0x05
#define BMM150_DATA_RATE_25HZ          0x06
#define BMM150_DATA_RATE_30HZ          0x07

/* XY重复次数 (精度) */
#define BMM150_REPXY_LOWPOWER          0x01    /* 3次  */
#define BMM150_REPXY_REGULAR           0x04    /* 9次  */
#define BMM150_REPXY_ENHANCED          0x07    /* 15次 */
#define BMM150_REPXY_HIGHACCURACY      0x17    /* 47次 */

/* Z轴重复次数 (精度) */
#define BMM150_REPZ_LOWPOWER           0x01    /* 3次  */
#define BMM150_REPZ_REGULAR            0x0E    /* 14次 */
#define BMM150_REPZ_ENHANCED           0x1B    /* 27次 */
#define BMM150_REPZ_HIGHACCURACY       0x39    /* 57次 */

/* 状态返回值 */
typedef enum {
    BMM150_OK = 0,
    BMM150_E_NULL_PTR,
    BMM150_E_DEV_NOT_FOUND,
    BMM150_E_INVALID_ARG,
    BMM150_E_COMM_FAIL,
    BMM150_E_SELF_TEST_FAIL,
    BMM150_E_NOT_READY
} bmm150_status_t;

/* 传感器数据结构 */
typedef struct {
    int16_t x;          /* X轴原始数据 */
    int16_t y;          /* Y轴原始数据 */
    int16_t z;          /* Z轴原始数据 */
    float x_uT;         /* X轴微特斯拉 */
    float y_uT;         /* Y轴微特斯拉 */
    float z_uT;         /* Z轴微特斯拉 */
    uint16_t data_ready;/* 数据就绪标志 */
} bmm150_data_t;

/* 配置结构 */
typedef struct {
    uint8_t op_mode;    /* 操作模式 */
    uint8_t data_rate;  /* 数据输出速率 */
    uint8_t xy_rep;     /* XY轴重复次数 */
    uint8_t z_rep;      /* Z轴重复次数 */
} bmm150_config_t;

/* 设备句柄结构 */
typedef struct {
    uint8_t i2c_addr;
    int16_t trim_data[4];   /* 校准数据 */
    bmm150_config_t config;
} bmm150_dev_t;

/* 函数声明 */

/**
 * @brief 初始化BMM150传感器
 * @param dev 设备句柄
 * @param i2c_addr I2C地址
 * @return 状态码
 */
bmm150_status_t bmm150_init(bmm150_dev_t *dev, uint8_t i2c_addr);

/**
 * @brief 软复位传感器
 * @param dev 设备句柄
 * @return 状态码
 */
bmm150_status_t bmm150_soft_reset(bmm150_dev_t *dev);

/**
 * @brief 设置操作模式
 * @param dev 设备句柄
 * @param op_mode 操作模式
 * @return 状态码
 */
bmm150_status_t bmm150_set_op_mode(bmm150_dev_t *dev, uint8_t op_mode);

/**
 * @brief 设置数据输出速率
 * @param dev 设备句柄
 * @param data_rate 数据速率
 * @return 状态码
 */
bmm150_status_t bmm150_set_data_rate(bmm150_dev_t *dev, uint8_t data_rate);

/**
 * @brief 设置XY轴重复次数
 * @param dev 设备句柄
 * @param rep_xy 重复次数设置
 * @return 状态码
 */
bmm150_status_t bmm150_set_xy_rep(bmm150_dev_t *dev, uint8_t rep_xy);

/**
 * @brief 设置Z轴重复次数
 * @param dev 设备句柄
 * @param rep_z 重复次数设置
 * @return 状态码
 */
bmm150_status_t bmm150_set_z_rep(bmm150_dev_t *dev, uint8_t rep_z);

/**
 * @brief 获取原始磁力数据
 * @param dev 设备句柄
 * @param data 数据结构指针
 * @return 状态码
 */
bmm150_status_t bmm150_get_raw_data(bmm150_dev_t *dev, bmm150_data_t *data);

/**
 * @brief 获取转换后的磁力数据(微特斯拉)
 * @param dev 设备句柄
 * @param data 数据结构指针
 * @return 状态码
 */
bmm150_status_t bmm150_get_mag_data(bmm150_dev_t *dev, bmm150_data_t *data);

/**
 * @brief 检查数据是否就绪
 * @param dev 设备句柄
 * @return 1:就绪 0:未就绪
 */
uint8_t bmm150_is_data_ready(bmm150_dev_t *dev);

/**
 * @brief 执行自检
 * @param dev 设备句柄
 * @return 状态码
 */
bmm150_status_t bmm150_self_test(bmm150_dev_t *dev);

/**
 * @brief 读取芯片ID
 * @param dev 设备句柄
 * @param chip_id 芯片ID指针
 * @return 状态码
 */
bmm150_status_t bmm150_get_chip_id(bmm150_dev_t *dev, uint8_t *chip_id);

#ifdef __cplusplus
}
#endif

#endif /* __BMM150_H */