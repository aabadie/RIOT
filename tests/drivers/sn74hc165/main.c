/*
 * SPDX-FileCopyrightText: 2026 Inria
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @ingroup     tests
 * @{
 *
 * @file
 * @brief       Test application for the SN74HC165 shift register driver
 *
 * @author      Alexandre Abadie <alexandre.abadie@inria.fr>
 *
 * @}
 */

#include <inttypes.h>
#include <stdio.h>

#include "sn74hc165.h"
#include "sn74hc165_params.h"
#include "ztimer.h"

#define POLL_MS     (20U)

static void _print_bits(uint32_t value, unsigned chain)
{
    for (int bit = (int)(8 * chain) - 1; bit >= 0; bit--) {
        putchar((value & (1UL << bit)) ? '1' : '0');
        if (bit % 8 == 0 && bit) {
            putchar(' ');
        }
    }
    putchar('\n');
}

int main(void)
{
    sn74hc165_t dev;

    puts("SN74HC165 shift register test application");
    int res = sn74hc165_init(&dev, &sn74hc165_params[0]);
    if (res != 0) {
        printf("Initialization failed: %d\n", res);
        return 1;
    }
    puts("Initialization OK, inputs are printed when they change (H..A)");

    uint32_t last = sn74hc165_read(&dev);
    _print_bits(last, dev.params.chain);

    while (1) {
        uint32_t value = sn74hc165_read(&dev);
        if (value != last) {
            printf("0x%08" PRIx32 " ", value);
            _print_bits(value, dev.params.chain);
            last = value;
        }
        ztimer_sleep(ZTIMER_MSEC, POLL_MS);
    }

    return 0;
}
