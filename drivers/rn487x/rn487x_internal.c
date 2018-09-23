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
 * @brief       Internal driver implementation for the RN4870/1 BLE
 *
 * @author      Alexandre Abadie <alexandre.abadie@inria.fr>
 *
 * @}
 */

#include <stdio.h>
#include <string.h>

#include "rn487x_internal.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

#define RESP_TIMEOUT_SEC            (5U)

static const char *closing_seq = "\r";

static void _uart_write_str(rn487x_t *dev, const char *str)
{
    size_t len = strlen(str);
    if (len) {
        uart_write(dev->p.uart, (uint8_t *)str, len);
    }
}

static void isr_resp_timeout(void *arg)
{
    rn487x_t *dev = (rn487x_t *)arg;
    mutex_unlock(&(dev->resp_lock));
}

static bool _wait_reply(rn487x_t *dev, uint8_t timeout)
{
    dev->resp_done = 0;
    dev->resp_size = 0;
    dev->resp_buf[0] = 0;

    xtimer_ticks64_t sent_time = xtimer_now64();

    xtimer_t resp_timer;
    resp_timer.callback = isr_resp_timeout;
    resp_timer.arg = dev;

    xtimer_set(&resp_timer, (uint32_t)timeout * US_PER_SEC);

    /* wait for results */
    while ((!dev->resp_done) &&
           xtimer_less(xtimer_diff32_64(xtimer_now64(), sent_time),
                                        xtimer_ticks_from_usec(timeout * US_PER_SEC))) {
        mutex_lock(&(dev->resp_lock));
    }

    xtimer_remove(&resp_timer);

    if (dev->resp_done == 0) {
        DEBUG("[rn487x] response timeout\n");
        return true;
    }

    return false;
}

int rn487x_write_cmd(rn487x_t *dev)
{
    int ret;
    DEBUG("[rn487x] CMD: %s\n", dev->cmd_buf);

    mutex_lock(&(dev->cmd_lock));
    _uart_write_str(dev, dev->cmd_buf);
    _uart_write_str(dev, closing_seq);

    ret = rn487x_wait_response(dev);
    if (ret == RN487X_TIMEOUT) {
        mutex_unlock(&(dev->cmd_lock));
        return RN487X_TIMEOUT;
    }

    mutex_unlock(&(dev->cmd_lock));

    ret = rn487x_process_response(dev);

    DEBUG("[rn487x] RET: %d, RESP: %s\n", ret, dev->resp_buf);

    return ret;
}

int rn487x_wait_response(rn487x_t *dev)
{
    if (_wait_reply(dev, RESP_TIMEOUT_SEC)) {
        return RN487X_TIMEOUT;
    }

    DEBUG("[rn487x] RESP: %s\n", dev->resp_buf);

    return RN487X_OK;
}

int rn487x_process_response(rn487x_t *dev)
{
    uint8_t ret = RN487X_DATA;
    if (strcmp(dev->resp_buf, "AOK") == 0) {
        DEBUG("[rn487x] command succeeded: '%s'\n", dev->cmd_buf);
        ret = RN487X_OK;
    }
    else if (strcmp(dev->resp_buf, "ERR") == 0) {
        DEBUG("[rn487x] command failed\n");
        ret = RN487X_ERR;
    }

    return ret;
}
