/*
 * Copyright (C)  2020 Inria
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     boards_adafruit-edgebadge
 * @{
 * @file
 * @brief       Board common implementations for the Adafruit EdgeBadge
 *
 * @author      Alexandre Abadie <alexandre.abadie@inria.fr>
 *
 * @}
 */

#include "cpu.h"
#include "board.h"
#include "periph/gpio.h"

void board_init(void)
{
    /* initialize the on-board "Yellow" LED */
    gpio_init(LED0_PIN, GPIO_OUT);

    /* initialize the CPU */
    cpu_init();
}
