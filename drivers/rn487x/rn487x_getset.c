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

#include "rn487x.h"
#include "rn487x_internal.h"

#define ENABLE_DEBUG    (0)
#include "debug.h"
