/*
 * Copyright (C) 2022 Inria
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     pkg_edge-impulse-sdk
 * @{
 *
 * @file
 * @brief       Edge Impulse Porting layer for DebugLog
 *
 * @author      Alexandre Abadie <alexandre.abadie@inria.fr>
 *
 * @}
 */

#include "ei_classifier_porting.h"

#include "edge-impulse-sdk/tensorflow/lite/micro/debug_log.h"
#include <stdio.h>
#include <stdarg.h>

#if defined(__cplusplus) && EI_C_LINKAGE == 1
extern "C"
#endif

void DebugLog(const char* s)
{
    ei_printf("%s", s);
}
