/**
 * @file bmm150.c
 * @brief BMM150 三轴地磁传感器驱动实现
 */

#include <stdint.h>
#include <stddef.h>
#include "bmm150.h"
#include "bmm150_platform.h"

/* 校准数据掩码和位置 */
#define BMM150_TRIM_XLSB_MASK       0x7F
#define BMM150_TRIM_YLSB_MASK       0x7F
#define BMM150_TRIM_ZLSB_MASK       0x7F
#define BMM150_TRIM_XYLSB_SHIFT     7

/* 校准系数 */
#define BMM150_LSB_TO_UT_FACTOR     16.0f   /* LSB到微特斯拉转换因子 */

/* 内部函数声明 */
static bmm150_status_t read_trim_data(bmm150_dev_t *dev);
static int16_t compensate_x(int16_t raw_x, bmm150_dev_t *dev);
static int16_t compensate_y(int16_t raw_y, bmm150_dev_t *dev);
static int16_t compensate_z(int16_t raw_z, bmm150_dev_t *dev);

/**
 * @brief 初始化BMM150传感器
 */
bmm150_status_t bmm150_init(bmm150_dev_t *dev, uint8_t i2c_addr)
{
    uint8_t chip_id = 0;
    bmm150_status_t status;

    if (dev == NULL) {
        return BMM150_E_NULL_PTR;
    }

    dev->i2c_addr = i2c_addr;

    /* 检查设备是否存在 */
    status = bmm150_get_chip_id(dev, &chip_id);
    if (status != BMM150_OK) {
        return status;
    }

    if (chip_id != BMM150_CHIP_ID) {
        return BMM150_E_DEV_NOT_FOUND;
    }

    /* 执行软复位 */
    status = bmm150_soft_reset(dev);
    if (status != BMM150_OK) {
        return status;
    }

    /* 读取校准数据 */
    status = read_trim_data(dev);
    if (status != BMM150_OK) {
        return status;
    }

    /* 设置默认配置 */
    dev->config.op_mode = BMM150_NORMAL_MODE;
    dev->config.data_rate = BMM150_DATA_RATE_10HZ;
    dev->config.xy_rep = BMM150_REPXY_REGULAR;
    dev->config.z_rep = BMM150_REPZ_REGULAR;

    /* 写入默认配置 */
    bmm150_set_data_rate(dev, dev->config.data_rate);
    bmm150_set_xy_rep(dev, dev->config.xy_rep);
    bmm150_set_z_rep(dev, dev->config.z_rep);
    bmm150_set_op_mode(dev, dev->config.op_mode);

    return BMM150_OK;
}

/**
 * @brief 软复位传感器
 */
bmm150_status_t bmm150_soft_reset(bmm150_dev_t *dev)
{
    uint8_t reg_data;
    bmm150_status_t status;

    if (dev == NULL) {
        return BMM150_E_NULL_PTR;
    }

    /* 写入软复位命令 */
    reg_data = BMM150_POWER_CNTRL_SOFT_RESET;
    status = bmm150_write_reg(dev, BMM150_POWER_CONTROL_REG, &reg_data, 1);
    if (status != BMM150_OK) {
        return status;
    }

    /* 等待复位完成 */
    bmm150_delay_ms(2);

    /* 检查复位是否完成 */
    do {
        status = bmm150_read_reg(dev, BMM150_POWER_CONTROL_REG, &reg_data, 1);
        if (status != BMM150_OK) {
            return status;
        }
    } while (reg_data & BMM150_POWER_CNTRL_SOFT_RESET);

    return BMM150_OK;
}

/**
 * @brief 设置操作模式
 */
bmm150_status_t bmm150_set_op_mode(bmm150_dev_t *dev, uint8_t op_mode)
{
    uint8_t reg_data;
    bmm150_status_t status;

    if (dev == NULL) {
        return BMM150_E_NULL_PTR;
    }

    status = bmm150_read_reg(dev, BMM150_OP_MODE_REG, &reg_data, 1);
    if (status != BMM150_OK) {
        return status;
    }

    reg_data = (reg_data & 0xE1) | (op_mode & 0x1E);

    status = bmm150_write_reg(dev, BMM150_OP_MODE_REG, &reg_data, 1);
    if (status == BMM150_OK) {
        dev->config.op_mode = op_mode;
    }

    return status;
}

/**
 * @brief 设置数据输出速率
 */
bmm150_status_t bmm150_set_data_rate(bmm150_dev_t *dev, uint8_t data_rate)
{
    uint8_t reg_data;
    bmm150_status_t status;

    if (dev == NULL) {
        return BMM150_E_NULL_PTR;
    }

    status = bmm150_read_reg(dev, BMM150_OP_MODE_REG, &reg_data, 1);
    if (status != BMM150_OK) {
        return status;
    }

    reg_data = (reg_data & 0x1F) | ((data_rate << 3) & 0xE0);

    status = bmm150_write_reg(dev, BMM150_OP_MODE_REG, &reg_data, 1);
    if (status == BMM150_OK) {
        dev->config.data_rate = data_rate;
    }

    return status;
}

/**
 * @brief 设置XY轴重复次数
 */
bmm150_status_t bmm150_set_xy_rep(bmm150_dev_t *dev, uint8_t rep_xy)
{
    bmm150_status_t status;

    if (dev == NULL) {
        return BMM150_E_NULL_PTR;
    }

    status = bmm150_write_reg(dev, BMM150_REP_XY_REG, &rep_xy, 1);
    if (status == BMM150_OK) {
        dev->config.xy_rep = rep_xy;
    }

    return status;
}

/**
 * @brief 设置Z轴重复次数
 */
bmm150_status_t bmm150_set_z_rep(bmm150_dev_t *dev, uint8_t rep_z)
{
    bmm150_status_t status;

    if (dev == NULL) {
        return BMM150_E_NULL_PTR;
    }

    status = bmm150_write_reg(dev, BMM150_REP_Z_REG, &rep_z, 1);
    if (status == BMM150_OK) {
        dev->config.z_rep = rep_z;
    }

    return status;
}

/**
 * @brief 检查数据是否就绪
 */
uint8_t bmm150_is_data_ready(bmm150_dev_t *dev)
{
    uint8_t reg_data;

    if (dev == NULL) {
        return 0;
    }

    if (bmm150_read_reg(dev, BMM150_DATA_READY_STATUS, &reg_data, 1) != BMM150_OK) {
        return 0;
    }

    return (reg_data & 0x01);
}

/**
 * @brief 获取原始磁力数据
 */
bmm150_status_t bmm150_get_raw_data(bmm150_dev_t *dev, bmm150_data_t *data)
{
    uint8_t buffer[8];
    bmm150_status_t status;

    if (dev == NULL || data == NULL) {
        return BMM150_E_NULL_PTR;
    }

    /* 如果是强制模式，需要先触发测量 */
    if (dev->config.op_mode == BMM150_FORCED_MODE) {
        status = bmm150_set_op_mode(dev, BMM150_FORCED_MODE);
        if (status != BMM150_OK) {
            return status;
        }
        /* 等待数据就绪 */
        while (!bmm150_is_data_ready(dev)) {
            bmm150_delay_ms(1);
        }
    }

    /* 读取数据寄存器 (0x42-0x48) */
    status = bmm150_read_reg(dev, BMM150_DATA_X_LSB, buffer, 8);
    if (status != BMM150_OK) {
        return status;
    }

    /* 解析X轴数据 (13位, 有符号) */
    data->x = (int16_t)(((uint16_t)buffer[1] << 8) | (buffer[0] & 0xF8));
    data->x = data->x >> 3;

    /* 解析Y轴数据 (13位, 有符号) */
    data->y = (int16_t)(((uint16_t)buffer[3] << 8) | (buffer[2] & 0xF8));
    data->y = data->y >> 3;

    /* 解析Z轴数据 (15位, 有符号) */
    data->z = (int16_t)(((uint16_t)buffer[5] << 8) | (buffer[4] & 0xFE));
    data->z = data->z >> 1;

    /* 数据就绪状态 */
    data->data_ready = buffer[7] & 0x01;

    return BMM150_OK;
}

/**
 * @brief 获取转换后的磁力数据(微特斯拉)
 */
bmm150_status_t bmm150_get_mag_data(bmm150_dev_t *dev, bmm150_data_t *data)
{
    bmm150_status_t status;
    int16_t compensated_x, compensated_y, compensated_z;

    if (dev == NULL || data == NULL) {
        return BMM150_E_NULL_PTR;
    }

    /* 获取原始数据 */
    status = bmm150_get_raw_data(dev, data);
    if (status != BMM150_OK) {
        return status;
    }

    /* 应用补偿 */
    compensated_x = compensate_x(data->x, dev);
    compensated_y = compensate_y(data->y, dev);
    compensated_z = compensate_z(data->z, dev);

    /* 转换为微特斯拉 */
    data->x_uT = (float)compensated_x * BMM150_LSB_TO_UT_FACTOR / 1000.0f;
    data->y_uT = (float)compensated_y * BMM150_LSB_TO_UT_FACTOR / 1000.0f;
    data->z_uT = (float)compensated_z * BMM150_LSB_TO_UT_FACTOR / 1000.0f;

    return BMM150_OK;
}

/**
 * @brief 读取芯片ID
 */
bmm150_status_t bmm150_get_chip_id(bmm150_dev_t *dev, uint8_t *chip_id)
{
    if (dev == NULL || chip_id == NULL) {
        return BMM150_E_NULL_PTR;
    }

    return bmm150_read_reg(dev, BMM150_CHIP_ID_REG, chip_id, 1);
}

/**
 * @brief 执行自检
 */
bmm150_status_t bmm150_self_test(bmm150_dev_t *dev)
{
    uint8_t reg_data;
    bmm150_status_t status;

    if (dev == NULL) {
        return BMM150_E_NULL_PTR;
    }

    /* 进入睡眠模式 */
    status = bmm150_set_op_mode(dev, BMM150_SLEEP_MODE);
    if (status != BMM150_OK) {
        return status;
    }

    bmm150_delay_ms(3);

    /* 启用自检 */
    reg_data = 0x01;
    status = bmm150_write_reg(dev, BMM150_BIST_CTRL_REG, &reg_data, 1);
    if (status != BMM150_OK) {
        return status;
    }

    bmm150_delay_ms(10);

    /* 检查自检结果 */
    status = bmm150_read_reg(dev, BMM150_BIST_STATUS_REG, &reg_data, 1);
    if (status != BMM150_OK) {
        return status;
    }

    /* 清除自检设置 */
    reg_data = 0x00;
    bmm150_write_reg(dev, BMM150_BIST_CTRL_REG, &reg_data, 1);

    /* 检查自检是否通过 */
    if (reg_data & 0x01) {
        return BMM150_OK;
    }

    return BMM150_E_SELF_TEST_FAIL;
}

/* ==================== 内部函数实现 ==================== */

/**
 * @brief 读取校准数据
 */
static bmm150_status_t read_trim_data(bmm150_dev_t *dev)
{
    uint8_t trim_xlsb, trim_xmsb;
    uint8_t trim_ylsb, trim_ymsb;
    uint8_t trim_zlsb, trim_zmsb;
    uint8_t trim_xy1_xy2;
    bmm150_status_t status;

    /* 读取X轴校准数据 */
    status = bmm150_read_reg(dev, BMM150_TRIM_XLSB_REG, &trim_xlsb, 1);
    if (status != BMM150_OK) return status;

    status = bmm150_read_reg(dev, BMM150_TRIM_XMSB_REG, &trim_xmsb, 1);
    if (status != BMM150_OK) return status;

    /* 读取Y轴校准数据 */
    status = bmm150_read_reg(dev, BMM150_TRIM_YLSB_REG, &trim_ylsb, 1);
    if (status != BMM150_OK) return status;

    status = bmm150_read_reg(dev, BMM150_TRIM_YMSB_REG, &trim_ymsb, 1);
    if (status != BMM150_OK) return status;

    /* 读取Z轴校准数据 */
    status = bmm150_read_reg(dev, BMM150_TRIM_ZLSB_REG, &trim_zlsb, 1);
    if (status != BMM150_OK) return status;

    status = bmm150_read_reg(dev, BMM150_TRIM_ZMSB_REG, &trim_zmsb, 1);
    if (status != BMM150_OK) return status;

    /* 读取XY校准数据 */
    status = bmm150_read_reg(dev, BMM150_TRIM_DATA_REG, &trim_xy1_xy2, 1);
    if (status != BMM150_OK) return status;

    /* 组合校准数据 */
    dev->trim_data[0] = (int16_t)((trim_xmsb << 8) | (trim_xlsb & BMM150_TRIM_XLSB_MASK));
    dev->trim_data[1] = (int16_t)((trim_ymsb << 8) | (trim_ylsb & BMM150_TRIM_YLSB_MASK));
    dev->trim_data[2] = (int16_t)((trim_zmsb << 8) | (trim_zlsb & BMM150_TRIM_ZLSB_MASK));
    dev->trim_data[3] = (int16_t)trim_xy1_xy2;

    return BMM150_OK;
}

/**
 * @brief X轴数据补偿
 */
static int16_t compensate_x(int16_t raw_x, bmm150_dev_t *dev)
{
    int16_t compensated_x;
    int16_t trim_x = dev->trim_data[0];
    uint16_t trim_xy1 = dev->trim_data[3] & 0x7F;
    uint16_t trim_xy2 = (dev->trim_data[3] >> 7) & 0x01;

    /* 简化的补偿算法 */
    compensated_x = raw_x - trim_x;

    /* 修正补偿 (BMM150数据手册中的补偿公式) */
    if (trim_xy1 != 0) {
        compensated_x = compensated_x + (int16_t)((trim_xy1 << 1) * (trim_xy2 ? 1 : -1));
    }

    return compensated_x;
}

/**
 * @brief Y轴数据补偿
 */
static int16_t compensate_y(int16_t raw_y, bmm150_dev_t *dev)
{
    int16_t compensated_y;
    int16_t trim_y = dev->trim_data[1];
    uint16_t trim_xy1 = dev->trim_data[3] & 0x7F;
    uint16_t trim_xy2 = (dev->trim_data[3] >> 7) & 0x01;

    /* 简化的补偿算法 */
    compensated_y = raw_y - trim_y;

    /* 修正补偿 */
    if (trim_xy1 != 0) {
        compensated_y = compensated_y + (int16_t)((trim_xy1 << 1) * (trim_xy2 ? -1 : 1));
    }

    return compensated_y;
}

/**
 * @brief Z轴数据补偿
 */
static int16_t compensate_z(int16_t raw_z, bmm150_dev_t *dev)
{
    int16_t compensated_z;
    int16_t trim_z = dev->trim_data[2];

    /* Z轴补偿 */
    compensated_z = raw_z - trim_z;

    return compensated_z;
}