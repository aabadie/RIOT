/*
 * SPDX-FileCopyrightText: 2026 Inria
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#pragma once

/**
 * @ingroup     drivers_adafruit_thermal
 * @{
 *
 * @file
 * @brief       Default parameters of the Adafruit thermal printer
 *
 * The heating and timing defaults are the ones of the Adafruit_Thermal
 * Arduino library.
 *
 * @author      Alexandre Abadie <alexandre.abadie@inria.fr>
 */

#include "adafruit_thermal.h"
#include "board.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    Default parameters
 * @{
 */
#ifndef ADAFRUIT_THERMAL_PARAM_UART
#  define ADAFRUIT_THERMAL_PARAM_UART           (UART_DEV(1))
#endif
#ifndef ADAFRUIT_THERMAL_PARAM_BAUDRATE
#  define ADAFRUIT_THERMAL_PARAM_BAUDRATE       (19200U)
#endif
#ifndef ADAFRUIT_THERMAL_PARAM_DTR_PIN
#  define ADAFRUIT_THERMAL_PARAM_DTR_PIN        (GPIO_UNDEF)
#endif
#ifndef ADAFRUIT_THERMAL_PARAM_HEAT_DOTS
#  define ADAFRUIT_THERMAL_PARAM_HEAT_DOTS      (11U)
#endif
#ifndef ADAFRUIT_THERMAL_PARAM_HEAT_TIME
#  define ADAFRUIT_THERMAL_PARAM_HEAT_TIME      (120U)
#endif
#ifndef ADAFRUIT_THERMAL_PARAM_HEAT_INTERVAL
#  define ADAFRUIT_THERMAL_PARAM_HEAT_INTERVAL  (40U)
#endif
#ifndef ADAFRUIT_THERMAL_PARAM_DENSITY
#  define ADAFRUIT_THERMAL_PARAM_DENSITY        (10U)
#endif
#ifndef ADAFRUIT_THERMAL_PARAM_BREAK_TIME
#  define ADAFRUIT_THERMAL_PARAM_BREAK_TIME     (2U)
#endif
#ifndef ADAFRUIT_THERMAL_PARAM_DOT_PRINT_US
#  define ADAFRUIT_THERMAL_PARAM_DOT_PRINT_US   (30000U)
#endif
#ifndef ADAFRUIT_THERMAL_PARAM_DOT_FEED_US
#  define ADAFRUIT_THERMAL_PARAM_DOT_FEED_US    (2100U)
#endif

#ifndef ADAFRUIT_THERMAL_PARAMS
#  define ADAFRUIT_THERMAL_PARAMS   { .uart = ADAFRUIT_THERMAL_PARAM_UART, \
                                      .baudrate = ADAFRUIT_THERMAL_PARAM_BAUDRATE, \
                                      .dtr_pin = ADAFRUIT_THERMAL_PARAM_DTR_PIN, \
                                      .heat_dots = ADAFRUIT_THERMAL_PARAM_HEAT_DOTS, \
                                      .heat_time = ADAFRUIT_THERMAL_PARAM_HEAT_TIME, \
                                      .heat_interval = ADAFRUIT_THERMAL_PARAM_HEAT_INTERVAL, \
                                      .density = ADAFRUIT_THERMAL_PARAM_DENSITY, \
                                      .break_time = ADAFRUIT_THERMAL_PARAM_BREAK_TIME, \
                                      .dot_print_us = ADAFRUIT_THERMAL_PARAM_DOT_PRINT_US, \
                                      .dot_feed_us = ADAFRUIT_THERMAL_PARAM_DOT_FEED_US }
#endif
/** @} */

/**
 * @brief   Parameters of the printers
 */
static const adafruit_thermal_params_t adafruit_thermal_params[] = {
    ADAFRUIT_THERMAL_PARAMS
};

#ifdef __cplusplus
}
#endif

/** @} */
