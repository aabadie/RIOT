/*
 * Copyright (C) 2021 Inria
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     tests
 * @{
 *
 * @file
 * @brief       Test application for the ft5336 touch driver
 *
 * @author      Alexandre Abadie <alexandre.abadie@inria.fr>
 *
 * @}
 */

#include <stdio.h>

#include "mutex.h"

#include "ft5336.h"
#include "ft5336_params.h"

static void _touch_event_cb(void *arg)
{
    mutex_unlock((mutex_t *)arg);
}


int main(void)
{
    mutex_t lock = MUTEX_INIT_LOCKED;
    ft5336_t dev;

    puts("FT5336 test application\n");

    printf("+------------Initializing------------+\n");
    int ret = ft5336_init(&dev, &ft5336_params[0], _touch_event_cb, &lock);
    if (ret != 0) {
        puts("[Error] Initialization failed");
        return 1;
    }

    puts("Initialization successful");

    ft5336_touch_state_t current_touch_state;
    ft5336_read_touch_state(&dev, &current_touch_state);
    ft5336_touch_state_t last_touch_state = current_touch_state;

    while (1) {

        /* wait for touch event */
        mutex_lock(&lock);

        ft5336_read_touch_state(&dev, &current_touch_state);

        if (current_touch_state != last_touch_state) {
            if (current_touch_state == FT5336_TOUCH_STATE_PRESSED) {
                puts("Pressed!");
            }
            if (current_touch_state == FT5336_TOUCH_STATE_RELEASED) {
                puts("Released!");
            }
            last_touch_state = current_touch_state;
        }

        /* Display touch position if pressed */
        if (current_touch_state == FT5336_TOUCH_STATE_PRESSED) {
            ft5336_touch_position_t position;
            ft5336_read_touch_position(&dev, &position);
            printf("X: %i, Y:%i\n", position.x, position.y);
        }
    }

    return 0;
}
