/*
 * SPDX-FileCopyrightText: 2026 Inria
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @ingroup     cpu_nrf54l
 * @{
 *
 * @file
 * @brief       Clock handling for the nRF54L family
 *
 * The CPU core and the basic peripherals run from the internal oscillators
 * and need no configuration. Only the HFXO request interface needed by the
 * radio and the LFCLK startup used by the low frequency peripherals (GRTC,
 * WDT) are implemented.
 *
 * @author      Alexandre Abadie <alexandre.abadie@inria.fr>
 *
 * @}
 */

#include <assert.h>

#include "cpu.h"
#include "irq.h"
#include "nrf_clock.h"

/* LFCLK source, the board can override this (e.g. CLOCK_LFCLK_SRC_SRC_LFRC
 * when no 32.768 kHz crystal is mounted) */
#ifndef CLOCK_LFCLK
#  define CLOCK_LFCLK       (CLOCK_LFCLK_SRC_SRC_LFXO)
#endif

static unsigned _hfxo_requests = 0;

void clock_start_lf(void)
{
    if (NRF_CLOCK->LFCLK.RUN) {
        /* LFCLK already running, nothing to do */
        return;
    }

    NRF_CLOCK->LFCLK.SRC = CLOCK_LFCLK;
    NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_LFCLKSTART = 1;
    while (!NRF_CLOCK->EVENTS_LFCLKSTARTED) {}
}

void clock_hfxo_request(void)
{
    unsigned state = irq_disable();
    ++_hfxo_requests;
    if (_hfxo_requests == 1) {
        NRF_CLOCK->EVENTS_XOSTARTED = 0;
        NRF_CLOCK->TASKS_XOSTART = 1;
        while (NRF_CLOCK->EVENTS_XOSTARTED == 0) {}
    }
    irq_restore(state);
}

void clock_hfxo_release(void)
{
    /* if this function is called while the counter is zero the state in a
     * driver requesting the HFXO is broken! */
    assert(_hfxo_requests);

    unsigned state = irq_disable();
    --_hfxo_requests;
    if (_hfxo_requests == 0) {
        NRF_CLOCK->TASKS_XOSTOP = 1;
    }
    irq_restore(state);
}
