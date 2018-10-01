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
 * @brief       Internal driver definitions for the RN4870/1 BLE device
 *
 * @author      Alexandre Abadie <alexandre.abadie@inria.fr>
 */

#ifndef RN487X_INTERNAL_H
#define RN487X_INTERNAL_H

#include <stdint.h>

#include "xtimer.h"
#include "rn487x.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RN487X_RESP_TIMEOUT            (500U * US_PER_MS)

/**
 * @brief   Process a command response
 *
 * @param[in] dev          The device descriptor
 *
 * @return                 RN487X_OK if the command succeeded
 * @return                 RN487X_ERR_* otherwise
 */
int rn487x_process_response(rn487x_t *dev);

#ifdef __cplusplus
}
#endif

#endif /* RN487X_INTERNAL_H */
/** @} */
