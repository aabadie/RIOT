#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "macros/units.h"
#include "cpu.h"
#include "periph/timer.h"

#define RFTIMER_MAX_COUNT           0xffffffff
#define MINIMUM_COMPAREVALE_ADVANCE 5
#define LARGEST_INTERVAL            0xffff

static timer_isr_ctx_t isr_ctx;

int timer_init(tim_t tim, uint32_t freq, timer_cb_t cb, void *arg)
{
    (void)tim;
    (void)freq;

    SCUM_RFTIMER->MAX_COUNT = RFTIMER_MAX_COUNT;
    // enable timer and interrupt
    SCUM_RFTIMER->CONTROL = RFTIMER_REG__CONTROL_ENABLE |
                            RFTIMER_REG__CONTROL_INTERRUPT_ENABLE |
                            RFTIMER_REG__CONTROL_COUNT_RESET;

    /* remember the interrupt context */
    isr_ctx.cb = cb;
    isr_ctx.arg = arg;

    NVIC_EnableIRQ(RFTIMER_IRQn);

    return 0;
}

#include <stdio.h>

int timer_set_absolute(tim_t tim, int channel, unsigned int value)
{
    (void)tim;

    SCUM_RFTIMER->INT_CLEAR = (uint32_t)(1 << channel);
    SCUM_RFTIMER->COMPARE[channel] = value * 2;
    SCUM_RFTIMER->COMPARE_CONTROL[channel] = RFTIMER_COMPARE_ENABLE | RFTIMER_COMPARE_INTERRUPT_ENABLE;

    return 0;
}

int timer_set(tim_t tim, int channel, unsigned int value)
{
    (void)tim;

    SCUM_RFTIMER->INT_CLEAR = (uint32_t)(1 << channel);
    SCUM_RFTIMER->COMPARE[channel] = SCUM_RFTIMER->COUNTER + value;
    SCUM_RFTIMER->COMPARE_CONTROL[channel] = RFTIMER_COMPARE_ENABLE | RFTIMER_COMPARE_INTERRUPT_ENABLE;

    return 0;
}

uword_t timer_query_freqs_numof(tim_t dev)
{
    (void)dev;
    return 1; // Only one frequency supported
}

uint32_t timer_query_freqs(tim_t dev, uword_t index)
{
    (void)dev;
    if (index > 0) {
        return 0;
    }

    return KHZ(500);
}

unsigned int timer_read(tim_t tim)
{
    (void)tim;
    return SCUM_RFTIMER->COUNTER;
}

int timer_clear(tim_t tim, int channel)
{
    (void)tim;

    SCUM_RFTIMER->INT_CLEAR = (uint32_t)(1 << channel);

    return 0;
}

void timer_start(tim_t tim)
{
    (void)tim;
    SCUM_RFTIMER->CONTROL = RFTIMER_REG__CONTROL_ENABLE |
                            RFTIMER_REG__CONTROL_INTERRUPT_ENABLE |
                            RFTIMER_REG__CONTROL_COUNT_RESET;
}

void timer_stop(tim_t tim)
{
    (void)tim;
    SCUM_RFTIMER->CONTROL |= RFTIMER_REG__CONTROL_COUNT_RESET;
    SCUM_RFTIMER->CONTROL &= ~RFTIMER_REG__CONTROL_ENABLE;
}

static void handle_compare_interrupt(uint8_t id) {
    isr_ctx.cb(isr_ctx.arg, id);
}

void RFTIMER_Handler(void) {
    uint32_t interrupt = SCUM_RFTIMER->INT;
    SCUM_RFTIMER->INT_CLEAR = interrupt;

    switch (interrupt) {
        case RFTIMER_REG__INT_COMPARE0_INT:
            handle_compare_interrupt(0);
            break;
        case RFTIMER_REG__INT_COMPARE1_INT:
            handle_compare_interrupt(1);
            break;
        case RFTIMER_REG__INT_COMPARE2_INT:
            handle_compare_interrupt(2);
            break;
        case RFTIMER_REG__INT_COMPARE3_INT:
            handle_compare_interrupt(3);
            break;
        case RFTIMER_REG__INT_COMPARE4_INT:
            handle_compare_interrupt(4);
            break;
        case RFTIMER_REG__INT_COMPARE5_INT:
            handle_compare_interrupt(5);
            break;
        case RFTIMER_REG__INT_COMPARE6_INT:
            handle_compare_interrupt(6);
            break;
        case RFTIMER_REG__INT_COMPARE7_INT:
            handle_compare_interrupt(7);
            break;
        default:
            break;
    }

    cortexm_isr_end();
}
