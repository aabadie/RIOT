/*
 * Copyright (C) 2018 Inria
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_rn487x
 * @{
 *
 * @file
 * @brief       Default configuration for RN4870/1 devices
 *
 * @author      Alexandre Abadie <alexandre.abadie@inria.fr>
 */

#ifndef RN487X_PARAMS_H
#define RN487X_PARAMS_H

#include "rn487x.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    Set default configuration parameters for the RN4870/1 driver
 * @{
 */
#ifndef RN487X_PARAM_UART
#define RN487X_PARAM_UART         UART_DEV(1)
#endif
#ifndef RN487X_PARAM_BAUDRATE
#define RN487X_PARAM_BAUDRATE     (115200U)
#endif
#ifndef RN487X_PARAM_PIN_RESET
#define RN487X_PARAM_PIN_RESET    (GPIO_UNDEF)
#endif
#ifndef RN487X_PARAM_PIN_WAKE
#define RN487X_PARAM_PIN_WAKE     (GPIO_UNDEF)
#endif

#ifndef RN487X_PARAMS
#define RN487X_PARAMS             { .uart      = RN487X_PARAM_UART,      \
                                    .baudrate  = RN487X_PARAM_BAUDRATE,  \
                                    .pin_reset = RN487X_PARAM_PIN_RESET, \
                                    .pin_wake  = RN487X_PARAM_PIN_WAKE }
#endif
/**@}*/

/**
 * @brief   RN4870/1 configuration
 */
static const rn487x_params_t rn487x_params[] =
{
    RN487X_PARAMS
};

#ifdef __cplusplus
}
#endif

#endif /* RN487X_PARAMS_H */
/** @} */
