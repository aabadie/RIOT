/*
 * Copyright (C)  2020 Inria
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     boards_arduino-nano-33-ble
 * @{
 * @file
 * @brief       Implementation for managing the Arduino bootloader
 *
 * @author      Alexandre Abadie <alexandre.abadie@inria.fr>
 *
 * @}
 */

#include "periph/pm.h"


#define NRF52_DOUBLE_TAP_MAGIC_NUMBER       (0x4e)

void reset_in_application(void)
{
    pm_reboot();
    while (1) {}
}

void reset_in_bootloader(void)
{
    NRF_POWER->GPREGRET = NRF52_DOUBLE_TAP_MAGIC_NUMBER;

    reset_in_application();
}
