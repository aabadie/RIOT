#include "ei_classifier_porting.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

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
 * @brief       Edge Impulse Porting layer for Classifier
 *
 * @author      Alexandre Abadie <alexandre.abadie@inria.fr>
 *
 * @}
 */

#include "timex.h"
#include "ztimer.h"

static char ei_print_buf[1024] = { 0 };

#define EI_WEAK_FN __attribute__((weak))

EI_WEAK_FN EI_IMPULSE_ERROR ei_run_impulse_check_canceled()
{
    return EI_IMPULSE_OK;
}

EI_WEAK_FN EI_IMPULSE_ERROR ei_sleep(int32_t time_ms)
{
    ztimer_sleep(ZTIMER_USEC, time_ms * US_PER_MS);
    return EI_IMPULSE_OK;
}

uint64_t ei_read_timer_ms()
{
    return ztimer_now(ZTIMER_USEC) / 1000;
}

uint64_t ei_read_timer_us()
{
    return ztimer_now(ZTIMER_USEC);
}

__attribute__((weak)) void ei_printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int r = vsnprintf(ei_print_buf, sizeof(ei_print_buf), format, args);
    va_end(args);

    if (r > 0) {
        printf("%s", ei_print_buf);
    }
}

__attribute__((weak)) void ei_printf_float(float f)
{
    printf("%f", f);
}

__attribute__((weak)) void *ei_malloc(size_t size)
{
    return malloc(size);
}

__attribute__((weak)) void *ei_calloc(size_t nitems, size_t size)
{
    return calloc(nitems, size);
}

__attribute__((weak)) void ei_free(void *ptr)
{
    free(ptr);
}

#if defined(__cplusplus) && EI_C_LINKAGE == 1
extern "C"
#endif

__attribute__((weak)) void DebugLog(const char* s)
{
    printf("%s", s);
}
