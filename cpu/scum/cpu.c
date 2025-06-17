/*
 * Copyright (C) 2025 Inria
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     cpu_nrf52
 * @{
 *
 * @file
 * @brief       Implementation of the CPU initialization
 *
 * @author      Alexandre Abadie <alexandre.abadie@inria.fr>
 *
 * @}
 */
#include <stdio.h>
#include "cpu.h"
#include "kernel_init.h"
#include "periph_conf.h"

#include "periph/init.h"

#include "optical.h"
#include "scm3c_hw_interface.h"

void post_startup(void)
{
    initialize_mote();
    perform_calibration();
}

void cpu_init(void)
{
    /* call cortexm default initialization */
    cortexm_init();

    /* initialize stdio prior to periph_init() to allow use of DEBUG() there */
    early_init();

    periph_init();
}
