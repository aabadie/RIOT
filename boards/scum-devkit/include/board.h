/*
 * Copyright (C) 2025 Inria
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#pragma once

/**
 * @ingroup     boards_scum-devkit
 * @{
 *
 * @file
 * @brief       Board specific definitions for the SCuM Dev Kit
 *
 * @author      Alexandre Abadie <alexandre.abadie@inria.fr>
 */

#include "cpu.h"
#include "periph_conf.h"
#include "periph_cpu.h"

#ifdef __cplusplus
extern "C" {
#endif

#define XTIMER_WIDTH                    (32U)

#define CONFIG_ZTIMER_USEC_BASE_FREQ    KHZ(500)
#define CONFIG_ZTIMER_USEC_WIDTH        (32U)

#ifdef __cplusplus
}
#endif

/** @} */
