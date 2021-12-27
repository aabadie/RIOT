/*
 * Copyright (C) 2021 Inria
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_ft5336
 *
 * @{
 * @file
 * @brief       Default configuration
 *
 * @author      Alexandre Abadie <alexandre.abadie@inria.fr>
 */

#ifndef FT5336_PARAMS_H
#define FT5336_PARAMS_H

#include <stdint.h>

#include "board.h"
#include "ft5336.h"
#include "ft5336_constants.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    Set default configuration parameters
 * @{
 */
/* I2C configuration */
#ifndef FT5336_PARAM_I2C_DEV
#define FT5336_PARAM_I2C_DEV            I2C_DEV(0)
#endif
#ifndef FT5336_PARAM_ADDR
#define FT5336_PARAM_ADDR               (FT5336_I2C_DEFAULT_ADDRESS)
#endif
#ifndef FT5336_PARAM_INT_PIN
#define FT5336_PARAM_INT_PIN            GPIO_UNDEF
#endif
#ifndef FT5336_PARAM_XMAX
#define FT5336_PARAM_XMAX               (480U)
#endif
#ifndef FT5336_PARAM_YMAX
#define FT5336_PARAM_YMAX               (272U)
#endif

#define FT5336_PARAMS   {               \
    .i2c = FT5336_PARAM_I2C_DEV,        \
    .addr = FT5336_PARAM_ADDR,          \
    .int_pin = FT5336_PARAM_INT_PIN,    \
    .xmax = FT5336_PARAM_XMAX,          \
    .ymax = FT5336_PARAM_YMAX,          \
}
/**@}*/

/**
 * @brief   Configuration struct
 */
static const ft5336_params_t ft5336_params[] =
{
    FT5336_PARAMS
};

/**
 * @brief   Default screen identifiers
 */
#ifndef FT5336_PARAM_SCREEN_IDS
#define FT5336_PARAM_SCREEN_IDS         0
#endif

/**
 * @brief   Configure screen identifiers
 */
static const uint8_t ft5336_screen_ids[] =
{
    FT5336_PARAM_SCREEN_IDS,
};

#ifdef __cplusplus
}
#endif

#endif /* FT5336_PARAMS_H */
/** @} */
