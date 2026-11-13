/*
 * SPDX-FileCopyrightText: 2026 Inria
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#pragma once

/**
 * @ingroup     drivers_sn74hc165
 * @{
 *
 * @file
 * @brief       Default configuration for the SN74HC165 shift register
 *
 * @author      Alexandre Abadie <alexandre.abadie@inria.fr>
 */

#include "board.h"
#include "sn74hc165.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    Default configuration parameters
 * @{
 */
#ifndef SN74HC165_PARAM_PL
#define SN74HC165_PARAM_PL      (GPIO_PIN(0, 0))    /**< parallel load pin */
#endif
#ifndef SN74HC165_PARAM_CLK
#define SN74HC165_PARAM_CLK     (GPIO_PIN(0, 1))    /**< clock pin */
#endif
#ifndef SN74HC165_PARAM_Q7
#define SN74HC165_PARAM_Q7      (GPIO_PIN(0, 2))    /**< serial output pin */
#endif
#ifndef SN74HC165_PARAM_CE
#define SN74HC165_PARAM_CE      (GPIO_UNDEF)        /**< clock enable pin */
#endif
#ifndef SN74HC165_PARAM_CHAIN
#define SN74HC165_PARAM_CHAIN   (1U)                /**< number of chips */
#endif

#ifndef SN74HC165_PARAMS
/** Default parameters */
#define SN74HC165_PARAMS        { .pl = SN74HC165_PARAM_PL,       \
                                  .clk = SN74HC165_PARAM_CLK,     \
                                  .q7 = SN74HC165_PARAM_Q7,       \
                                  .ce = SN74HC165_PARAM_CE,       \
                                  .chain = SN74HC165_PARAM_CHAIN }
#endif
/** @} */

/**
 * @brief   Configuration of the shift register chains
 */
static const sn74hc165_params_t sn74hc165_params[] = {
    SN74HC165_PARAMS
};

#ifdef __cplusplus
}
#endif

/** @} */
