/*
 * Copyright (C) Inria 2025
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#pragma once

/**
 * @ingroup   boards_scum-devkit
 * @{
 *
 * @file
 * @brief     Board specific configuration of direct mapped GPIOs
 *
 * @author    Alexandre Abadie <alexandre.abadie@inria.fr>
 */

#include "board.h"
#include "saul/periph.h"

#ifdef __cplusplus
extern "C" {
#endif


#define SAUL_GPIO_NUMOF (0U)

/**
 * @brief    GPIO pin configuration
 */
static const  saul_gpio_params_t saul_gpio_params[] = {
    {}
};

#ifdef __cplusplus
}
#endif

/** @} */
