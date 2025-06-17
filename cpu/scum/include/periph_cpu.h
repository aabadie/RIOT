/*
 * Copyright (C) 2025 Inria
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#pragma once

/**
 * @ingroup         cpu_scum
 * @{
 *
 * @file
 * @brief           SCuM specific definitions for handling peripherals
 *
 * @author          Alexandre Abadie <alexandre.abadie@inria.fr>
 */

#include <stdint.h>
#include "macros/units.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   System core clock speed, fixed to 5MHz
 */
#define CLOCK_CORECLOCK         MHZ(5)

#define PROVIDES_PM_SET_LOWEST_CORTEXM

#define UART_NUMOF              (1U)

#define PERIPH_TIMER_PROVIDES_SET
#define TIMER_NUMOF             (1U)
#define TIMER_CHANNEL_NUMOF     (8U)

#ifdef __cplusplus
}
#endif

/** @} */
