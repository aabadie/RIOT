/*
 * Copyright (C) 2018 Inria
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_rn487x
 * @{
 *
 * @file
 * @brief       Driver implementation for the RN4870/1 BLE device
 *
 * @author      Alexandre Abadie <alexandre.abadie@inria.fr>
 *
 * @}
 */

#include <string.h>
#include <errno.h>

#include "assert.h"
#include "xtimer.h"
#include "fmt.h"
#include "rn487x_params.h"
#include "rn487x.h"
#include "rn487x_internal.h"

#define ENABLE_DEBUG    (0)
/* Warning: to correctly display the debug message from sleep timer callback,,
   add CFLAGS+=-DTHREAD_STACKSIZE_IDLE=THREAD_STACKSIZE_DEFAULT to the build
   command.
*/
#include "debug.h"

/**
 * @brief   Delay when resetting the device, 10ms
 */
#define RESET_DELAY                 (10UL * US_PER_MS)

/*
 * Interrupt callbacks
 */
static void _rx_cb(void *arg, uint8_t c)
{
    rn487x_t *dev = (rn487x_t *)arg;
    netdev_t *netdev = (netdev_t *)dev;

    (void)dev;
    (void)netdev;
    (void)c;
}

/*
 * Driver's "public" functions
 */
void rn487x_setup(rn487x_t *dev, const rn487x_params_t *params)
{
    assert(dev && (params->uart < UART_NUMOF));

    /* initialize device parameters */
    memcpy(&dev->p, params, sizeof(rn487x_params_t));

    /* UART is initialized later, since interrupts cannot be handled yet */
}

int rn487x_init(rn487x_t *dev)
{
    /* initialize UART and GPIO pins */
    if (uart_init(dev->p.uart, dev->p.baudrate, _rx_cb, dev) != UART_OK) {
        DEBUG("[rn487x] init: error initializing UART\n");
        return -ENXIO;
    }

    DEBUG("[rn487x] init: initialization successful\n");
    return RN487X_OK;
}

int rn487x_reset(rn487x_t *dev)
{
    (void) dev;

    return RN487X_OK;
}

int rn487x_factory_reset(rn487x_t *dev)
{
    size_t p = snprintf(dev->cmd_buf, sizeof(dev->cmd_buf) - 1, "SF,1");
    dev->cmd_buf[p] = 0;
    int ret = rn487x_write_cmd(dev);
    if (ret == RN487X_TIMEOUT) {
        DEBUG("[rn487x] factory reset: failed\n");
        return ret;
    }

    return RN487X_OK;
}
