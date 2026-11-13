/*
 * SPDX-FileCopyrightText: 2026 Inria
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @ingroup     drivers_sn74hc165
 * @{
 *
 * @file
 * @brief       SN74HC165 driver implementation
 *
 * @author      Alexandre Abadie <alexandre.abadie@inria.fr>
 *
 * @}
 */

#include <errno.h>

#include "ztimer.h"
#include "sn74hc165.h"

#define ENABLE_DEBUG 0
#include "debug.h"

static inline void _wait(void)
{
    if (CONFIG_SN74HC165_DELAY_US) {
        ztimer_spin(ZTIMER_USEC, CONFIG_SN74HC165_DELAY_US);
    }
}

int sn74hc165_init(sn74hc165_t *dev, const sn74hc165_params_t *params)
{
    if (params->chain == 0 || params->chain > SN74HC165_CHAIN_MAX) {
        DEBUG("[sn74hc165] invalid chain length %u\n", params->chain);
        return -EINVAL;
    }
    dev->params = *params;

    if (gpio_init(params->pl, GPIO_OUT) || gpio_init(params->clk, GPIO_OUT) ||
        gpio_init(params->q7, GPIO_IN)) {
        DEBUG("[sn74hc165] GPIO init failed\n");
        return -EIO;
    }
    gpio_set(params->pl);
    gpio_clear(params->clk);

    if (gpio_is_valid(params->ce)) {
        if (gpio_init(params->ce, GPIO_OUT)) {
            DEBUG("[sn74hc165] CE GPIO init failed\n");
            return -EIO;
        }
        /* only enable the clock while reading */
        gpio_set(params->ce);
    }

    return 0;
}

uint32_t sn74hc165_read(const sn74hc165_t *dev)
{
    const sn74hc165_params_t *p = &dev->params;
    uint32_t value = 0;

    /* sample the inputs */
    gpio_clear(p->pl);
    _wait();
    gpio_set(p->pl);
    _wait();

    if (gpio_is_valid(p->ce)) {
        gpio_clear(p->ce);
    }

    /* the first chip comes out first, each chip from H (D7) down to A (D0) */
    for (unsigned chip = 0; chip < p->chain; chip++) {
        for (int bit = 7; bit >= 0; bit--) {
            if (gpio_read(p->q7)) {
                value |= 1UL << (8 * chip + bit);
            }
            gpio_set(p->clk);
            _wait();
            gpio_clear(p->clk);
            _wait();
        }
    }

    if (gpio_is_valid(p->ce)) {
        gpio_set(p->ce);
    }

    return value;
}
