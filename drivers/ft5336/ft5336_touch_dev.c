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
 * @brief       Driver adaption to touch_dev generic interface
 *
 * @author      Alexandre Abadie <alexandre.abadie@inria.fr>
 * @}
 */

#include <stddef.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>

#include "kernel_defines.h"

#include "ft5336.h"
#include "ft5336_touch_dev.h"

#define ENABLE_DEBUG 0
#include "debug.h"

uint16_t _ft5336_height(const touch_dev_t *touch_dev)
{
    const ft5336_t *dev = (const ft5336_t *)touch_dev;
    assert(dev);

    return dev->params.ymax;
}

uint16_t _ft5336_width(const touch_dev_t *touch_dev)
{
    const ft5336_t *dev = (const ft5336_t *)touch_dev;
    assert(dev);

    return dev->params.xmax;
}

uint8_t _ft5336_touches(const touch_dev_t *touch_dev, touch_t *touches, size_t len)
{
    (void)len;

    ft5336_t *dev = (ft5336_t *)touch_dev;
    assert(dev);

    ft5336_touch_state_t state;
    ft5336_read_touch_state(dev, &state);
    uint8_t ret = (state == FT5336_TOUCH_STATE_PRESSED);

    if (ret && touches != NULL) {
        ft5336_touch_position_t pos;
        ft5336_read_touch_position(dev, &pos);
        touches[0].x = pos.x;
        touches[0].y = pos.y;

        DEBUG("X: %i, Y: %i\n", touches[0].x, touches[0].y);
    }

    return ret;
}

void _ft5336_set_event_callback(const touch_dev_t *touch_dev, touch_event_cb_t cb, void *arg)
{
    ft5336_t *dev = (ft5336_t *)touch_dev;
    assert(dev);

    dev->cb = (ft5336_event_cb_t)cb;
    dev->cb_arg = arg;
}

const touch_dev_driver_t ft5336_touch_dev_driver = {
    .height     = _ft5336_height,
    .width      = _ft5336_width,
    .touches    = _ft5336_touches,
    .set_event_callback = _ft5336_set_event_callback,
};
