/*
 * Copyright (C) 2021 Inria
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    drivers_ft5336 FT5336 touch panel driver
 * @ingroup     drivers_sensors
 * @brief       Multi-touch capacitive panel controller
 *
 * @{
 *
 * @file
 *
 * @author      Alexandre Abadie <alexandre.abadie@inria.fr>
 */

#ifndef FT5336_H
#define FT5336_H

#include <stdint.h>

#include "periph/gpio.h"
#include "periph/i2c.h"

#include "ft5336_constants.h"

#ifdef MODULE_TOUCH_DEV
#include "touch_dev.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Touch state enum
 */
typedef enum {
    FT5336_TOUCH_STATE_PRESSED,         /**< Touchscreen is pressed */
    FT5336_TOUCH_STATE_RELEASED,        /**< Touchscreen is released */
} ft5336_touch_state_t;

/**
 * @brief  Touch position structure
 */
typedef struct {
    uint16_t x;                         /**< X position */
    uint16_t y;                         /**< Y position */
} ft5336_touch_position_t;

/**
 * @brief   Signature of the touch event callback triggered from interrupt
 *
 * @param[in] arg           optional context for the callback
 */
typedef void (*ft5336_event_cb_t)(void *arg);

/**
 * @brief   Device initialization parameters
 */
typedef struct {
    i2c_t i2c;              /**< I2C device which is used */
    uint8_t addr;           /**< Device I2C address */
    gpio_t int_pin;         /**< Touch screen interrupt pin */
    uint16_t xmax;          /**< Touch screen max X position */
    uint16_t ymax;          /**< Touch screen max Y position */
} ft5336_params_t;

/**
 * @brief   Device descriptor for the driver
 */
typedef struct {
#ifdef MODULE_TOUCH_DEV
    touch_dev_t *dev;       /**< Pointer to the generic touch device */
#endif
    ft5336_params_t params; /**< Initialization parameters */
    ft5336_event_cb_t cb;   /**< Configured IRQ event callback */
    void *cb_arg;           /**< Extra argument for the callback */
} ft5336_t;

/**
 * @brief   Initialize the given device
 *
 * @param[inout] dev        Device descriptor of the driver
 * @param[in] params        Initialization parameters
 * @param[in] cb            Callback function called on touch interrupts
 * @param[in] arg           Context argument used in callback function
 *
 * @return                  0 on success
 * @return                  -ENODEV when no valid device
 * @return                  -EPROTO on any bus error
 */
int ft5336_init(ft5336_t *dev, const ft5336_params_t *params,
                ft5336_event_cb_t cb, void *arg);

/**
 * @brief   Read the touch position
 *
 * @param[in] dev           Device descriptor of the FT5336
 * @param[out] position     Touch position
 *
 * @return                  0 on success
 * @return                  -EPROTO on any bus error
 */
int ft5336_read_touch_position(const ft5336_t *dev, ft5336_touch_position_t *position);

/**
 * @brief   Read the touch state (pressed or released)
 *
 * @param[in] dev           Device descriptor of the FT5336
 * @param[out] state        Touch state
 *
 * @return                  0 on success
 * @return                  -EPROTO on any bus error
 */
int ft5336_read_touch_state(const ft5336_t *dev, ft5336_touch_state_t *state);

#ifdef __cplusplus
}
#endif

#endif /* FT5336_H */
/** @} */
