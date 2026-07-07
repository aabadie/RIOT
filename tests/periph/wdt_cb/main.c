/*
 * SPDX-FileCopyrightText: 2026 Inria
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @ingroup tests
 * @{
 *
 * @file
 * @brief       Test for the periph_wdt_cb feature
 *
 *              This test configures the watchdog with an early warning
 *              callback and lets it expire without being kicked. The
 *              callback prints a message, then the watchdog resets the MCU.
 *
 * @author      Alexandre Abadie <alexandre.abadie@inria.fr>
 *
 * @}
 */

#include <stdio.h>

#include "periph/wdt.h"

/* reset time, long enough to let the startup messages be printed */
#define WDT_TIMEOUT_MS      (1024U)

static void wdt_cb(void *arg)
{
    (void)arg;

    puts("SUCCESS");
}

int main(void)
{
    puts("RIOT wdt_cb test application");

    wdt_setup_reboot_with_callback(0, WDT_TIMEOUT_MS, wdt_cb, NULL);
    printf("The watchdog will expire in %ums and reboot the MCU\n",
           WDT_TIMEOUT_MS);
    wdt_start();

    /* wait for the watchdog to expire, without kicking it */
    while (1) {}

    return 0;
}
