/*
 * Copyright (C) 2021 Inria
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     sys_auto_init
 * @{
 * @file
 * @brief       initializes ft5336 display device
 *
 * @author      Alexandre Abadie <alexandre.abadie@inria.fr>
 * @}
 */

#include <stddef.h>

#include "log.h"

#include "touch_dev.h"

#include "ft5336.h"
#include "ft5336_params.h"
#include "ft5336_touch_dev.h"

#define FT5336_NUMOF           ARRAY_SIZE(ft5336_params)

ft5336_t ft5336_devs[FT5336_NUMOF];
static touch_dev_reg_t touch_dev_entries[FT5336_NUMOF];

void auto_init_ft5336(void)
{
    assert(FT5336_NUMOF == ARRAY_SIZE(ft5336_screen_ids));

    for (size_t i = 0; i < FT5336_NUMOF; i++) {
        LOG_DEBUG("[auto_init_screen] initializing ft5336 #%u\n", i);
        if (ft5336_init(&ft5336_devs[i], &ft5336_params[i], NULL, NULL) < 0) {
            LOG_ERROR("[auto_init_screen] error initializing ft5336 #%u\n", i);
            continue;
        }

        touch_dev_entries[i].dev = (touch_dev_t *)&ft5336_devs[i];
        touch_dev_entries[i].screen_id = ft5336_screen_ids[i];
        touch_dev_entries[i].dev->driver = &ft5336_touch_dev_driver;

        /* add to touch_dev registry */
        touch_dev_reg_add(&(touch_dev_entries[i]));
    }
}
