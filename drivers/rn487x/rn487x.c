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
#include "isrpipe.h"
#include "rn487x_params.h"
#include "rn487x.h"
#include "rn487x_internal.h"

#define ENABLE_DEBUG    (1)
/* Warning: to correctly display the debug message from sleep timer callback,,
   add CFLAGS+=-DTHREAD_STACKSIZE_IDLE=THREAD_STACKSIZE_DEFAULT to the build
   command.
*/
#include "debug.h"

#ifndef RN487X_DEBUG_INCOMING
#define RN487X_DEBUG_INCOMING (1)
#endif

/**
 * @brief   Delay when resetting the device, 10ms
 */
#define RESET_DELAY                 (10UL * US_PER_MS)

/*
 * Interrupt callbacks
 */
// static void _rx_cb(void *arg, uint8_t c)
// {
//     rn487x_t *dev = (rn487x_t *)arg;
//     netdev_t *netdev = (netdev_t *)dev;

//     // printf("%c", c);

//     /* Avoid overflow of module response buffer */
//     if (dev->resp_size >= RN487X_MAX_BUF) {
//         return;
//     }

//     /* The device closes the response with \r\n when done */
//     if (strncmp(dev->resp_buf, "CMD>", 4) == 0) {
//         dev->resp_buf[dev->resp_size++] = '\0';
//         dev->resp_size = 0;
//         dev->resp_done = true;
//         puts("");
//         mutex_unlock(&(dev->resp_lock));
//     }
//     else {
//         dev->resp_buf[dev->resp_size++] = c;
//     }

//     (void)netdev;
// }

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
    isrpipe_init(&dev->isrpipe, dev->buf, RN487X_MAX_BUF);
    if (uart_init(dev->p.uart, dev->p.baudrate, (uart_rx_cb_t) isrpipe_write_one,
                  &dev->isrpipe) != UART_OK) {
        return -ENXIO;
    }

    /* initialize pins and perform hardware reset */
    if (dev->p.pin_reset != GPIO_UNDEF) {
        DEBUG("[rn487x] setup: initialize reset pin\n");
        gpio_init(dev->p.pin_reset, GPIO_OUT);
        gpio_clear(dev->p.pin_reset);
        xtimer_usleep(RESET_DELAY);
        gpio_set(dev->p.pin_reset);
        xtimer_usleep(500 * US_PER_MS);
    }

    /* if wake pin is connected, pull it low */
    if (dev->p.pin_wake != GPIO_UNDEF) {
        DEBUG("[rn487x] setup: initialize wake pin\n");
        gpio_init(dev->p.pin_wake, GPIO_OUT);
        gpio_clear(dev->p.pin_wake);
        xtimer_usleep(5 * US_PER_MS);
    }

    // char buf[8] = "%REBOOT%";
    // int res = isrpipe_read_all_timeout(&dev->isrpipe, buf, 8, RN487X_RESP_TIMEOUT);
    // if (res != 8) {
    if (rn487x_wait_until(dev, "%REBOOT%", RN487X_RESP_TIMEOUT) < 0) {
        DEBUG("[rn487x] device wake up failed\n");
        return -RN487X_ERR;
    }

    dev->mode = RN487X_DATA_MODE;

    rn487x_set_command_mode(dev);

    DEBUG("[rn487x] init: initialization successful\n");
    return RN487X_OK;
}

int rn487x_wait_until(rn487x_t *dev, const char *expected, uint32_t timeout)
{
    while (*expected) {
        char c;
        int res;
        if ((res = isrpipe_read_timeout(&dev->isrpipe, &c, 1, timeout)) == 1) {
            if (RN487X_DEBUG_INCOMING) {
                printf("%c", c);
            }
            if (c != *expected++) {
                return -1;
            }
        }
        else {
            return res;
        }
    }

    return 0;
}

int rn487x_reset(rn487x_t *dev)
{
    if (dev->mode != RN487X_COMMAND_MODE) {
        DEBUG("[rn487x] reset: switch to command mode\n");
        rn487x_set_command_mode(dev);
    }

    size_t p = snprintf(dev->buf, sizeof(dev->buf) - 1, "R,1\r");
    dev->buf[p] = 0;
    size_t len = strlen(dev->buf);
    uart_write(dev->p.uart, (uint8_t *)dev->buf, len);
    if (rn487x_wait_until(dev, " Rebooting\r\n", RN487X_RESP_TIMEOUT) < 0) {
        DEBUG("[rn487x] device reset failed\n");
        return -RN487X_ERR;
    }

    dev->mode = RN487X_DATA_MODE;
    return RN487X_OK;
}

int rn487x_factory_reset(rn487x_t *dev)
{
    if (dev->mode != RN487X_COMMAND_MODE) {
        DEBUG("[rn487x] factory_reset: switch to command mode\n");
        rn487x_set_command_mode(dev);
    }

    size_t p = snprintf(dev->buf, sizeof(dev->buf) - 1, "SF,1\r");
    dev->buf[p] = 0;
    size_t len = strlen(dev->buf);
    uart_write(dev->p.uart, (uint8_t *)dev->buf, len);
    if (rn487x_wait_until(dev, " %REBOOT%", RN487X_RESP_TIMEOUT) < 0) {
        DEBUG("[rn487x] device reset failed\n");
        return -RN487X_ERR;
    }

    dev->mode = RN487X_DATA_MODE;
    return RN487X_OK;
}
