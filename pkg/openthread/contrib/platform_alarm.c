/*
 * SPDX-FileCopyrightText: 2017 Fundacion Inria Chile
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @{
 * @ingroup     net
 * @file
 * @brief       Implementation of OpenThread alarm platform abstraction
 *
 * The millisecond alarm runs on ZTIMER_MSEC, the microsecond alarm (timers
 * of the SubMac, OPENTHREAD_CONFIG_PLATFORM_USEC_TIMER_ENABLE) on
 * ZTIMER_USEC: both post an event to the OpenThread event queue.
 *
 * With ztimer_ondemand, a clock only runs while it is acquired, and its times
 * are only comparable while it runs (a stopped timer may restart from 0, e.g.
 * on nRF5x). The millisecond clock is the time base of OpenThread (timeouts,
 * ages of the neighbors): it is acquired while OpenThread runs, on the RTT of
 * the boards which have one. The microsecond clock is acquired from the first
 * microsecond alarm to otPlatAlarmMicroStop(), called by OpenThread when it
 * has no microsecond timer left: it runs during the CSMA backoffs, the ACK
 * timeouts and the delays before a retransmission timed by OpenThread, and
 * during an energy scan.
 *
 * @author      Jose Ignacio Alamos <jialamos@uc.cl>
 * @}
 */

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>

#include "event.h"
#include "openthread/platform/alarm-micro.h"
#include "openthread/platform/alarm-milli.h"
#include "ot.h"
#include "ztimer.h"

#define ENABLE_DEBUG 0
#include "debug.h"

static void _milli_handler(event_t *event)
{
    (void)event;
    otPlatAlarmMilliFired(openthread_get_instance());
}

static void _micro_handler(event_t *event)
{
    (void)event;
    otPlatAlarmMicroFired(openthread_get_instance());
}

static event_t _ev_milli = { .handler = _milli_handler };
static event_t _ev_micro = { .handler = _micro_handler };

static void _timeout_cb(void *arg)
{
    event_post(openthread_get_evq(), arg);
}

static ztimer_t _timer_milli = { .callback = _timeout_cb, .arg = &_ev_milli };
static ztimer_t _timer_micro = { .callback = _timeout_cb, .arg = &_ev_micro };

/* the microsecond clock is acquired while OpenThread has microsecond timers */
static bool _micro_acquired;

/* fires @p event @p dt ticks of @p clock after @p t0 (the times of
 * OpenThread are the 32 bit ticks of the clock, modulo 2^32) */
static void _start_at(ztimer_clock_t *clock, ztimer_t *timer, event_t *event,
                      uint32_t t0, uint32_t dt)
{
    /* time already elapsed since t0, none if t0 is after the current time:
     * read before the clock was restarted by ztimer_ondemand */
    int32_t elapsed = (int32_t)(ztimer_now(clock) - t0);

    if (elapsed < 0) {
        elapsed = 0;
    }

    if ((uint32_t)elapsed >= dt) {
        ztimer_remove(clock, timer);
        event_post(openthread_get_evq(), event);
    }
    else {
        ztimer_set(clock, timer, dt - elapsed);
    }
}

/* a stopped alarm doesn't fire, even if its event was posted already */
static void _stop(ztimer_clock_t *clock, ztimer_t *timer, event_t *event)
{
    ztimer_remove(clock, timer);
    event_cancel(openthread_get_evq(), event);
}

void openthread_alarm_init(void)
{
    ztimer_acquire(ZTIMER_MSEC);
}

void otPlatAlarmMilliStartAt(otInstance *aInstance, uint32_t aT0, uint32_t aDt)
{
    (void)aInstance;
    DEBUG("openthread: otPlatAlarmMilliStartAt: aT0: %" PRIu32 ", aDt: %" PRIu32 "\n", aT0, aDt);
    _start_at(ZTIMER_MSEC, &_timer_milli, &_ev_milli, aT0, aDt);
}

void otPlatAlarmMilliStop(otInstance *aInstance)
{
    (void)aInstance;
    _stop(ZTIMER_MSEC, &_timer_milli, &_ev_milli);
}

uint32_t otPlatAlarmMilliGetNow(void)
{
    return ztimer_now(ZTIMER_MSEC);
}

void otPlatAlarmMicroStartAt(otInstance *aInstance, uint32_t aT0, uint32_t aDt)
{
    (void)aInstance;
    DEBUG("openthread: otPlatAlarmMicroStartAt: aT0: %" PRIu32 ", aDt: %" PRIu32 "\n", aT0, aDt);
    if (!_micro_acquired) {
        ztimer_acquire(ZTIMER_USEC);
        _micro_acquired = true;
    }
    _start_at(ZTIMER_USEC, &_timer_micro, &_ev_micro, aT0, aDt);
}

void otPlatAlarmMicroStop(otInstance *aInstance)
{
    (void)aInstance;
    _stop(ZTIMER_USEC, &_timer_micro, &_ev_micro);
    if (_micro_acquired) {
        ztimer_release(ZTIMER_USEC);
        _micro_acquired = false;
    }
}

uint32_t otPlatAlarmMicroGetNow(void)
{
    /* without a microsecond timer of OpenThread the clock may be stopped:
     * the time read then is only used to start a timer right after */
    ztimer_acquire(ZTIMER_USEC);
    uint32_t now = ztimer_now(ZTIMER_USEC);
    ztimer_release(ZTIMER_USEC);

    return now;
}
