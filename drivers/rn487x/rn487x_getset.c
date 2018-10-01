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
 * @file
 * @brief       Implementation of get and set functions for RN4870/1
 *
 * @author      Alexandre Abadie <alexandre.abadie@inria.fr>
 * @}
 */

#include <string.h>
#include <errno.h>

#include "assert.h"
#include "isrpipe.h"

#include "rn487x.h"
#include "rn487x_internal.h"

#include "periph/uart.h"

#define ENABLE_DEBUG    (1)
#include "debug.h"

int rn487x_set_data_mode(rn487x_t *dev)
{
    if (dev->mode == RN487X_DATA_MODE) {
        return 0;
    }

    char mode_str[4] = "---\r";
    uart_write(dev->p.uart, (uint8_t *)mode_str, 4);
    if (rn487x_wait_until(dev, "END", RN487X_RESP_TIMEOUT) < 0) {
        DEBUG("[rn487x] set data mode failed\n");
        return -1;
    }

    DEBUG("[rn487x] set data mode succeeded\n");
    dev->mode = RN487X_DATA_MODE;

    return 0;
}

int rn487x_set_command_mode(rn487x_t *dev)
{
    if (dev->mode == RN487X_COMMAND_MODE) {
        return 0;
    }

    char mode_str[3] = "$$$";
    uart_write(dev->p.uart, (uint8_t *)mode_str, 3);
    if (rn487x_wait_until(dev, "CMD>", RN487X_RESP_TIMEOUT) < 0) {
        DEBUG("[rn487x] set command mode failed\n");
        return -1;
    }

    DEBUG("[rn487x] set command mode succeeded\n");

    dev->mode = RN487X_COMMAND_MODE;

    return 0;
}

int rn487x_set_mode(rn487x_t *dev, rn487x_mode_t mode)
{
    if (mode == RN487X_DATA_MODE) {
        return rn487x_set_data_mode(dev);
    }
    else if (mode == RN487X_DATA_MODE) {
        return rn487x_set_command_mode(dev);
    }

    DEBUG("[rn487x] invalid mode '%d'\n", (uint8_t)mode);
    return -1;
    // uart_write(dev->p.uart, (uint8_t *)mode_str, 3);
    // int res = isrpipe_read_all_timeout(&dev->isrpipe, "CMD>", 1, timeout);


    // if ((rn487x_wait_response(dev) != RN487X_TIMEOUT) &&
    //         (strcmp(dev->resp_buf, "CMD>") == 0)) {
    // }
    // uart_write(dev->p.uart, (uint8_t *)"+\r", 2);
    // uart_write(dev->p.uart, (uint8_t *)"SS,C0\r", 6);
    // uart_write(dev->p.uart, (uint8_t *)"F\r", 2);
    // uart_write(dev->p.uart, (uint8_t *)"C,0,001E64FEF398\r", 17);
}