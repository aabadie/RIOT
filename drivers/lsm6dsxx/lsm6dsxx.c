/*
 * SPDX-FileCopyrightText: 2017 OTA keys S.A.
 * SPDX-FileCopyrightText: 2024 HAW Hamburg
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @ingroup     drivers_lsm6dsxx
 * @{
 *
 * @file
 * @brief       Device driver implementation for the LSM6DSXX 3D accelerometer/gyroscope.
 *
 * @author      Vincent Dupont <vincent@otakeys.com>
 * @author      Sebastian Meiling <s@mlng.net>
 * @author      Miquel Borrell <miquel.borrell@haw-hamburg.de>
 *
 * @}
 */

#include <assert.h>

#include "ztimer.h"
#include "macros/utils.h"

#include "lsm6dsxx.h"
#include "lsm6dsxx_internal.h"

#define ENABLE_DEBUG 0
#include "debug.h"

#define BUS         (dev->params.i2c)
#define ADDR        (dev->params.addr)

/**
 * order in array [0, 1, 2, 3] is
 * LSM6DSXX_ACC_FS_2G, LSM6DSXX_ACC_FS_16G, LSM6DSXX_ACC_FS_4G, LSM6DSXX_ACC_FS_8G
 */
static const int16_t range_acc[] = { 2000, 16000, 4000, 8000 };

/**
 * order in array [0, 1, 2, 3] is
 * LSM6DSXX_GYRO_FS_245DPS, LSM6DSXX_GYRO_FS_500DPS,
 * LSM6DSXX_GYRO_FS_1000DPS, LSM6DSXX_GYRO_FS_2000DPS
 */
static const int16_t range_gyro[] = { 2450, 5000, 10000, 20000 };

int lsm6dsxx_init(lsm6dsxx_t *dev, const lsm6dsxx_params_t *params)
{
    uint8_t tmp;
    int res;

    assert(dev && params);

    dev->params = *params;

    i2c_acquire(BUS);

    /* Reboot */
    i2c_write_reg(BUS, ADDR, LSM6DSXX_REG_CTRL3_C, LSM6DSXX_CTRL3_C_BOOT, 0);

    ztimer_sleep(ZTIMER_MSEC, LSM6DSXX_BOOT_WAIT_MS);

    if (i2c_read_reg(BUS, ADDR, LSM6DSXX_REG_WHO_AM_I, &tmp, 0) < 0) {
        i2c_release(BUS);
        DEBUG("[ERROR] lsm6dsxx_init: i2c_read_reg LSM6DSXX_REG_WHO_AM_I!\n");
        return -LSM6DSXX_ERROR_BUS;
    }

    if (tmp == LSM6DS33_WHO_AM_I) {
        dev->temperature_scaling_factor = 4;
    }
    else if (tmp == LSM6DSL_WHO_AM_I) {
        dev->temperature_scaling_factor = 8;
    }
    else {
        i2c_release(BUS);
        DEBUG("[ERROR] lsm6dsxx_init: WHO_AM_I\n");
        return -LSM6DSXX_ERROR_DEV;
    }

    /* Block data update: the two bytes of a value come from the same sample,
       and the address increments in a burst read */
    res = i2c_write_reg(BUS, ADDR, LSM6DSXX_REG_CTRL3_C,
                        LSM6DSXX_CTRL3_C_BDU | LSM6DSXX_CTRL3_C_IF_INC, 0);
    /* Set acc odr / full scale */
    tmp = (dev->params.acc_odr << LSM6DSXX_CTRL_ODR_SHIFT) |
          (dev->params.acc_fs << LSM6DSXX_CTRL_FS_SHIFT);
    res += i2c_write_reg(BUS, ADDR, LSM6DSXX_REG_CTRL1_XL, tmp, 0);
    /* Set gyro odr / full scale */
    tmp = (dev->params.gyro_odr << LSM6DSXX_CTRL_ODR_SHIFT) |
          (dev->params.gyro_fs << LSM6DSXX_CTRL_FS_SHIFT);
    res += i2c_write_reg(BUS, ADDR, LSM6DSXX_REG_CTRL2_G, tmp, 0);
    /* Set continuous mode */
    uint8_t fifo_odr = MAX(dev->params.acc_odr, dev->params.gyro_odr);
    tmp = (fifo_odr << LSM6DSXX_FIFO_CTRL5_FIFO_ODR_SHIFT) |
          LSM6DSXX_FIFO_CTRL5_CONTINUOUS_MODE;
    res += i2c_write_reg(BUS, ADDR, LSM6DSXX_REG_FIFO_CTRL5, tmp, 0);
    tmp = (dev->params.gyro_decimation << LSM6DSXX_FIFO_CTRL3_GYRO_DEC_SHIFT) |
          dev->params.acc_decimation;
    res += i2c_write_reg(BUS, ADDR, LSM6DSXX_REG_FIFO_CTRL3, tmp, 0);

    i2c_release(BUS);

    if (res < 0) {
        DEBUG("[ERROR] lsm6dsxx_init: config\n");
        return -LSM6DSXX_ERROR_CNF;
    }
    return LSM6DSXX_OK;
}

int lsm6dsxx_read_acc(const lsm6dsxx_t *dev, lsm6dsxx_3d_data_t *data)
{
    i2c_acquire(BUS);
    uint8_t buf[6];
    /* x, y and z in one burst, little endian */
    int res = i2c_read_regs(BUS, ADDR, LSM6DSXX_REG_OUTX_L_XL, buf, sizeof(buf), 0);
    i2c_release(BUS);
    data->x = (int16_t)(buf[0] | (buf[1] << 8));
    data->y = (int16_t)(buf[2] | (buf[3] << 8));
    data->z = (int16_t)(buf[4] | (buf[5] << 8));

    if (res < 0) {
        DEBUG("[ERROR] lsm6dsxx_read_acc\n");
        return -LSM6DSXX_ERROR_BUS;
    }

    assert(dev->params.acc_fs < LSM6DSXX_ACC_FS_MAX);
    data->x = ((int32_t)data->x * range_acc[dev->params.acc_fs]) / INT16_MAX;
    data->y = ((int32_t)data->y * range_acc[dev->params.acc_fs]) / INT16_MAX;
    data->z = ((int32_t)data->z * range_acc[dev->params.acc_fs]) / INT16_MAX;

    return LSM6DSXX_OK;
}

int lsm6dsxx_read_gyro(const lsm6dsxx_t *dev, lsm6dsxx_3d_data_t *data)
{
    i2c_acquire(BUS);
    uint8_t buf[6];
    /* x, y and z in one burst, little endian */
    int res = i2c_read_regs(BUS, ADDR, LSM6DSXX_REG_OUTX_L_G, buf, sizeof(buf), 0);
    i2c_release(BUS);
    data->x = (int16_t)(buf[0] | (buf[1] << 8));
    data->y = (int16_t)(buf[2] | (buf[3] << 8));
    data->z = (int16_t)(buf[4] | (buf[5] << 8));

    if (res < 0) {
        DEBUG("[ERROR] lsm6dsxx_read_gyro\n");
        return -LSM6DSXX_ERROR_BUS;
    }

    assert(dev->params.gyro_fs < LSM6DSXX_GYRO_FS_MAX);
    data->x = ((int32_t)data->x * range_gyro[dev->params.gyro_fs]) / INT16_MAX;
    data->y = ((int32_t)data->y * range_gyro[dev->params.gyro_fs]) / INT16_MAX;
    data->z = ((int32_t)data->z * range_gyro[dev->params.gyro_fs]) / INT16_MAX;

    return LSM6DSXX_OK;
}

int lsm6dsxx_read_temp(const lsm6dsxx_t *dev, int16_t *data)
{
    uint8_t tmp;
    uint16_t traw;
    /* read raw temperature */
    i2c_acquire(BUS);
    if (i2c_read_reg(BUS, ADDR, LSM6DSXX_REG_OUT_TEMP_L, &tmp, 0) < 0) {
        i2c_release(BUS);
        return -LSM6DSXX_ERROR_BUS;
    }
    traw = tmp;
    if (i2c_read_reg(BUS, ADDR, LSM6DSXX_REG_OUT_TEMP_H, &tmp, 0) < 0) {
        i2c_release(BUS);
        return -LSM6DSXX_ERROR_BUS;
    }
    traw |= (uint16_t)tmp << 8;
    i2c_release(BUS);
    /* convert temperature to degC x 100 */
    traw += LSM6DSXX_TEMP_OFFSET << dev->temperature_scaling_factor;
    *data = (int16_t)(((int32_t)traw * 100) >> dev->temperature_scaling_factor);

    return LSM6DSXX_OK;
}

int lsm6dsxx_acc_power_down(const lsm6dsxx_t *dev)
{
    int res;
    uint8_t tmp;

    i2c_acquire(BUS);
    res = i2c_read_reg(BUS, ADDR, LSM6DSXX_REG_CTRL1_XL, &tmp, 0);
    if (res < 0) {
        i2c_release(BUS);
        DEBUG("[ERROR] lsm6dsxx_acc_power_down\n");
        return -LSM6DSXX_ERROR_BUS;
    }

    tmp &= ~(LSM6DSXX_CTRL_ODR_MASK);
    res = i2c_write_reg(BUS, ADDR, LSM6DSXX_REG_CTRL1_XL, tmp, 0);

    i2c_release(BUS);

    if (res < 0) {
        DEBUG("[ERROR] lsm6dsxx_acc_power_down\n");
        return -LSM6DSXX_ERROR_BUS;
    }

    return LSM6DSXX_OK;
}

int lsm6dsxx_gyro_power_down(const lsm6dsxx_t *dev)
{
    int res;
    uint8_t tmp;

    i2c_acquire(BUS);
    res = i2c_read_reg(BUS, ADDR, LSM6DSXX_REG_CTRL2_G, &tmp, 0);
    if (res < 0) {
        i2c_release(BUS);
        DEBUG("[ERROR] lsm6dsxx_gyro_power_down\n");
        return -LSM6DSXX_ERROR_BUS;
    }

    tmp &= ~(LSM6DSXX_CTRL_ODR_MASK);
    res = i2c_write_reg(BUS, ADDR, LSM6DSXX_REG_CTRL2_G, tmp, 0);

    i2c_release(BUS);

    if (res < 0) {
        DEBUG("[ERROR] lsm6dsxx_gyro_power_down\n");
        return -LSM6DSXX_ERROR_BUS;
    }

    return LSM6DSXX_OK;
}

int lsm6dsxx_acc_power_up(const lsm6dsxx_t *dev)
{
    int res;
    uint8_t tmp;

    i2c_acquire(BUS);
    res = i2c_read_reg(BUS, ADDR, LSM6DSXX_REG_CTRL1_XL, &tmp, 0);
    if (res < 0) {
        i2c_release(BUS);
        DEBUG("[ERROR] lsm6dsxx_acc_power_up\n");
        return -LSM6DSXX_ERROR_BUS;
    }

    tmp &= ~(LSM6DSXX_CTRL_ODR_MASK);
    tmp |= dev->params.acc_odr << LSM6DSXX_CTRL_ODR_SHIFT;
    res = i2c_write_reg(BUS, ADDR, LSM6DSXX_REG_CTRL1_XL, tmp, 0);

    i2c_release(BUS);

    if (res < 0) {
        DEBUG("[ERROR] lsm6dsxx_acc_power_up\n");
        return -LSM6DSXX_ERROR_BUS;
    }

    return LSM6DSXX_OK;
}

int lsm6dsxx_gyro_power_up(const lsm6dsxx_t *dev)
{
    int res;
    uint8_t tmp;

    i2c_acquire(BUS);
    res = i2c_read_reg(BUS, ADDR, LSM6DSXX_REG_CTRL2_G, &tmp, 0);
    if (res < 0) {
        i2c_release(BUS);
        DEBUG("[ERROR] lsm6dsxx_gyro_power_up\n");
        return -LSM6DSXX_ERROR_BUS;
    }

    tmp &= ~(LSM6DSXX_CTRL_ODR_MASK);
    tmp |= dev->params.gyro_odr << LSM6DSXX_CTRL_ODR_SHIFT;
    res = i2c_write_reg(BUS, ADDR, LSM6DSXX_REG_CTRL2_G, tmp, 0);

    i2c_release(BUS);

    if (res < 0) {
        DEBUG("[ERROR] lsm6dsxx_gyro_power_up\n");
        return -LSM6DSXX_ERROR_BUS;
    }

    return LSM6DSXX_OK;
}
