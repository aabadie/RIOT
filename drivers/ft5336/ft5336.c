/*
 * Copyright (C) 2021 Inria
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_ft5336
 * @{
 *
 * @file
 * @brief       Device driver implementation for the FT5336 touch driver
 *
 * @author      Alexandre Abadie <alexandre.abadie@inria.fr>
 *
 * @}
 */

#include <errno.h>
#include <stdint.h>

#include "periph/i2c.h"
#include "periph/gpio.h"

#include "ztimer.h"

#include "ft5336.h"
#include "ft5336_constants.h"
#include "ft5336_params.h"

#define ENABLE_DEBUG    0
#include "debug.h"

#define FT5336_BUS              (dev->params.i2c)
#define FT5336_ADDR             (dev->params.addr)

#define FT5336_RESET_DELAY_MS   (200)

static void _gpio_irq(void *arg)
{
    const ft5336_t *dev = (const ft5336_t *)arg;

    assert(dev);

    if (dev->cb) {
        dev->cb(dev->cb_arg);
    }
}

int ft5336_init(ft5336_t *dev, const ft5336_params_t *params, ft5336_event_cb_t cb, void *arg)
{
    dev->params = *params;
    dev->cb = cb;
    dev->cb_arg = arg;

    /* Wait at least 200ms after power up before accessing registers */
    ztimer_sleep(ZTIMER_MSEC, FT5336_RESET_DELAY_MS);

    i2c_acquire(FT5336_BUS);

    uint8_t dev_id = 0;
    if (i2c_read_reg(FT5336_BUS, FT5336_ADDR, FT5336_CHIP_ID_REG, &dev_id, 0) != 0) {
        i2c_release(FT5336_BUS);
        return -EPROTO;
    }

    if (dev_id != FT5336_CHIP_ID) {
        DEBUG("[ft5336] init: invalid chip ID: '0x%02x' (expected: 0x%02x)\n",
                dev_id, FT5336_CHIP_ID);
        i2c_release(FT5336_BUS);
        return -ENODEV;
    }

    /* Configure interrupt */
    if (gpio_is_valid(dev->params.int_pin)) {
        DEBUG("[ft5336] init: configuring touchscreen interrupt\n");
        gpio_init_int(dev->params.int_pin, GPIO_IN, GPIO_RISING, _gpio_irq, dev);
        i2c_write_reg(FT5336_BUS, FT5336_ADDR, FT5336_G_MODE_REG, FT5336_G_MODE_INTERRUPT_TRIGGER & 0x01, 0);
    }

    i2c_release(FT5336_BUS);

    return 0;
}

int ft5336_read_touch_position(const ft5336_t *dev, ft5336_touch_position_t *position)
{
    uint8_t reg = 0;
    uint16_t pos_x = 0, pos_y = 0;
    i2c_acquire(FT5336_BUS);
    i2c_read_reg(FT5336_BUS, FT5336_ADDR, FT5336_TOUCH1_XL_REG, &reg, 0);
    pos_x = reg & FT5336_TOUCH_POS_LSB_MASK;
    i2c_read_reg(FT5336_BUS, FT5336_ADDR, FT5336_TOUCH1_XH_REG, &reg, 0);
    pos_x |= (uint16_t)(reg & FT5336_TOUCH_POS_MSB_MASK) << 8;
    i2c_read_reg(FT5336_BUS, FT5336_ADDR, FT5336_TOUCH1_YL_REG, &reg, 0);
    pos_y = reg & FT5336_TOUCH_POS_LSB_MASK;
    i2c_read_reg(FT5336_BUS, FT5336_ADDR, FT5336_TOUCH1_YH_REG, &reg, 0);
    pos_y |= (uint16_t)(reg & FT5336_TOUCH_POS_MSB_MASK) << 8;
    i2c_release(FT5336_BUS);

    /* X and Y positions are swapped compared to the display */
    position->x = pos_y;
    position->y = pos_x;

    return 0;
}

int ft5336_read_touch_state(const ft5336_t *dev, ft5336_touch_state_t *state)
{
    uint8_t touches_count = 0;
    i2c_acquire(FT5336_BUS);
    i2c_read_reg(FT5336_BUS, FT5336_ADDR, FT5336_TD_STATUS_REG, &touches_count, 0);
    i2c_release(FT5336_BUS);
    touches_count &= FT5336_TD_STATUS_MASK;

    if (touches_count > FT5336_TOUCHES_COUNT_MAX) {
        touches_count = 0;
    }

    if (touches_count > 0) {
        *state = FT5336_TOUCH_STATE_PRESSED;
    }
    else {
        *state = FT5336_TOUCH_STATE_RELEASED;
    }

    return 0;
}
