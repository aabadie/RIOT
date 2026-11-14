/*
 * SPDX-FileCopyrightText: 2026 Inria
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @ingroup     cpu_samd5x
 * @ingroup     drivers_periph_pdm
 * @{
 *
 * @file
 * @brief       PDM microphone on a SERCOM in SPI master mode
 *
 * The SAMD5x has no PDM peripheral and its I2S peripheral is not wired to
 * the microphone of boards like the Adafruit EdgeBadge. A SERCOM in SPI
 * master mode clocks the microphone instead: its SCK pin is the PDM clock
 * and its MISO pin reads the PDM data, 32 bits at a time.
 *
 * The data register empty interrupt keeps the clock running and passes each
 * 32-bit word to a decimation filter in software: a 128-tap Hann window,
 * one sample out for every 64 bits in. Its zeros fall on the multiples of
 * the output sample rate, where the noise of the microphone would alias
 * back to low frequencies. A one pole high-pass filter then removes the DC
 * offset. At 16 kHz, the interrupt runs 31250 times per second.
 *
 * Only mono is supported: the SERCOM samples the data on one clock edge.
 *
 * @author      Alexandre Abadie <alexandre.abadie@inria.fr>
 * @}
 */

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>

#include "board.h"
#include "cpu.h"
#include "periph_conf.h"
#include "periph/gpio.h"
#include "periph/pdm.h"

#define ENABLE_DEBUG 0
#include "debug.h"

#if !defined(PDM_MIC_CLK_MIN_HZ) || !defined(PDM_MIC_CLK_MAX_HZ)
#  error "PDM: Define PDM_MIC_CLK_MIN_HZ and PDM_MIC_CLK_MAX_HZ for the microphone used."
#endif

#if PDM_MIC_CLK_MIN_HZ > PDM_MIC_CLK_MAX_HZ
#  error "PDM: Minimum PDM clock is higher than maximum!"
#endif

/* PDM bits per output sample */
#define PDM_DECIMATION      (64U)

/* Pole of the DC removal filter, Q15: 0.995, a cut-off of about 13 Hz at
   16 kHz */
#define PDM_DC_POLE         (32604)

/* Gain applied after filtering, Q8 */
#define PDM_GAIN_UNITY      (256)

/* 128-tap Hann window, 0.5 - 0.5 * cos(2 * pi * (i + 0.5) / 128), scaled
   to a sum of 65536 */
static const uint16_t _taps[2 * PDM_DECIMATION] = {
       0,    1,    4,    8,   12,   19,   26,   34,   44,   55,   67,   79,
      93,  108,  124,  141,  159,  178,  197,  217,  238,  260,  282,  305,
     328,  351,  375,  400,  424,  449,  474,  499,  525,  550,  575,  600,
     624,  649,  673,  696,  719,  742,  764,  786,  807,  827,  846,  865,
     883,  900,  916,  931,  945,  957,  969,  980,  990,  998, 1005, 1012,
    1016, 1020, 1023, 1024, 1024, 1023, 1020, 1016, 1012, 1005,  998,  990,
     980,  969,  957,  945,  931,  916,  900,  883,  865,  846,  827,  807,
     786,  764,  742,  719,  696,  673,  649,  624,  600,  575,  550,  525,
     499,  474,  449,  424,  400,  375,  351,  328,  305,  282,  260,  238,
     217,  197,  178,  159,  141,  124,  108,   93,   79,   67,   55,   44,
      34,   26,   19,   12,    8,    4,    1,    0,
};

static pdm_isr_ctx_t _isr_ctx;
static int16_t _buf[2][PDM_BUF_SIZE];
static uint8_t _buf_cur;
static uint16_t _buf_pos;
static int32_t _gain_q8 = PDM_GAIN_UNITY;

/* decimation filter state: each word feeds the sample in progress and the
   next one, the windows of two samples overlap by half */
static uint32_t _acc_cur;
static uint32_t _acc_next;
static bool _odd_word;

/* DC removal filter state */
static int32_t _dc_in;
static int32_t _dc_out;

static SercomSpi *_dev(void)
{
    return pdm_config.dev;
}

/* sum of the taps where the word has a bit set, the first bit received in
   bit 0 (LSB first) */
static uint32_t _dot(uint32_t word, const uint16_t *taps)
{
    uint32_t sum = 0;

    while (word) {
        sum += taps[__builtin_ctz(word)];
        word &= word - 1;
    }
    return sum;
}

static void _output(uint32_t acc)
{
    /* half of the bits set is silence */
    int32_t in = (int32_t)acc - 32768;
    int32_t out = in - _dc_in + ((_dc_out * PDM_DC_POLE) >> 15);

    _dc_in = in;
    _dc_out = out;

    int32_t sample = (out * _gain_q8) >> 8;
    if (sample > INT16_MAX) {
        sample = INT16_MAX;
    }
    else if (sample < INT16_MIN) {
        sample = INT16_MIN;
    }
    _buf[_buf_cur][_buf_pos++] = (int16_t)sample;
    if (_buf_pos == PDM_BUF_SIZE) {
        _buf_pos = 0;
        _buf_cur ^= 1;
        if (_isr_ctx.cb) {
            _isr_ctx.cb(_isr_ctx.arg, _buf[_buf_cur ^ 1]);
        }
    }
}

static void _filter_word(uint32_t word)
{
    /* the sample in progress gets the second half of the window, the next
       sample its first half */
    unsigned half = _odd_word ? 32 : 0;

    _acc_cur += _dot(word, &_taps[PDM_DECIMATION + half]);
    _acc_next += _dot(word, &_taps[half]);
    if (_odd_word) {
        _output(_acc_cur);
        _acc_cur = _acc_next;
        _acc_next = 0;
    }
    _odd_word = !_odd_word;
}

void PDM_ISR(void)
{
    SercomSpi *dev = _dev();

    /* the word received while the previous one was sent, then the next
       word to send, which keeps the clock running */
    uint32_t word = dev->DATA.reg;
    dev->DATA.reg = 0;
    _filter_word(word);
    cortexm_isr_end();
}

static uint32_t _gclk_hz(void)
{
    return sam0_gclk_freq(pdm_config.gclk_src);
}

/* SPI clock = gclk / (2 * (BAUD + 1)), the closest to the requested clock
   inside the clock range of the microphone */
static uint8_t _baud(uint32_t rate)
{
    uint32_t gclk = _gclk_hz();
    uint32_t clk = rate * PDM_DECIMATION;
    uint32_t baud_min = (gclk + 2 * PDM_MIC_CLK_MAX_HZ - 1) / (2 * PDM_MIC_CLK_MAX_HZ);
    uint32_t baud_max = gclk / (2 * PDM_MIC_CLK_MIN_HZ);
    uint32_t baud = (gclk + clk) / (2 * clk);

    baud_min = baud_min ? baud_min - 1 : 0;
    baud_max = baud_max ? baud_max - 1 : 0;
    baud = baud ? baud - 1 : 0;
    baud = baud < baud_min ? baud_min : baud;
    baud = baud > baud_max ? baud_max : baud;
    return baud > UINT8_MAX ? UINT8_MAX : (uint8_t)baud;
}

static int32_t _gain(int8_t gain_db)
{
    /* 10 ^ (1 / 20) is about 1.122 per dB */
    int32_t q8 = PDM_GAIN_UNITY;

    if (gain_db > PDM_GAIN_MAX) {
        gain_db = PDM_GAIN_MAX;
    }
    if (gain_db < PDM_GAIN_MIN) {
        gain_db = PDM_GAIN_MIN;
    }
    for (int i = 0; i < gain_db; i++) {
        q8 = q8 * 1122 / 1000;
    }
    for (int i = 0; i > gain_db; i--) {
        q8 = q8 * 1000 / 1122;
    }
    return q8;
}

int32_t pdm_init(pdm_mode_t mode, uint32_t rate, int8_t gain,
                 pdm_data_cb_t cb, void *arg)
{
    SercomSpi *dev = _dev();

    if (mode != PDM_MODE_MONO) {
        DEBUG("[pdm] init: mode not supported\n");
        return -ENOTSUP;
    }
    /* the lowest rate for 0 */
    rate = rate ? rate : 1;

    _isr_ctx.cb = cb;
    _isr_ctx.arg = arg;
    _gain_q8 = _gain(gain);

    gpio_init(pdm_config.clk_pin, GPIO_OUT);
    gpio_clear(pdm_config.clk_pin);
    gpio_init_mux(pdm_config.clk_pin, pdm_config.clk_mux);
    gpio_init(pdm_config.data_pin, GPIO_IN);
    gpio_init_mux(pdm_config.data_pin, pdm_config.data_mux);

    sercom_clk_en(dev);
    sercom_set_gen(dev, pdm_config.gclk_src);

    dev->CTRLA.reg = SERCOM_SPI_CTRLA_SWRST;
    while (dev->CTRLA.reg & SERCOM_SPI_CTRLA_SWRST) {}

    uint8_t baud = _baud(rate);
    /* master, LSB first, SPI mode 0: the data is read on the rising edge
       of the clock, the edge a mono microphone answers on when its select
       pin is low */
    dev->CTRLA.reg = SERCOM_SPI_CTRLA_MODE(0x3)
                   | SERCOM_SPI_CTRLA_DOPO(pdm_config.clk_pad)
                   | SERCOM_SPI_CTRLA_DIPO(pdm_config.data_pad)
                   | SERCOM_SPI_CTRLA_DORD;
    dev->CTRLB.reg = SERCOM_SPI_CTRLB_RXEN;
    while (dev->SYNCBUSY.reg) {}
    dev->CTRLC.reg = SERCOM_SPI_CTRLC_DATA32B;
    dev->BAUD.reg = baud;

    uint32_t clk = _gclk_hz() / (2 * (baud + 1));
    int32_t real_rate = clk / PDM_DECIMATION;
    DEBUG("[pdm] PDM clock %lu Hz, sample rate %ld Hz\n", (unsigned long)clk, (long)real_rate);

    /* the interrupt must keep up with the clock, give it the highest
       priority */
    IRQn_Type irq = (IRQn_Type)(SERCOM0_0_IRQn + 4 * sercom_id(dev));
    NVIC_SetPriority(irq, 0);
    NVIC_EnableIRQ(irq);

    return real_rate;
}

void pdm_start(void)
{
    SercomSpi *dev = _dev();

    _acc_cur = 0;
    _acc_next = 0;
    _odd_word = false;
    _dc_in = 0;
    _dc_out = 0;
    _buf_cur = 0;
    _buf_pos = 0;

    dev->CTRLA.reg |= SERCOM_SPI_CTRLA_ENABLE;
    while (dev->SYNCBUSY.reg) {}
    dev->INTENSET.reg = SERCOM_SPI_INTENSET_DRE;
}

void pdm_stop(void)
{
    SercomSpi *dev = _dev();

    dev->INTENCLR.reg = SERCOM_SPI_INTENCLR_DRE;
    dev->CTRLA.reg &= ~SERCOM_SPI_CTRLA_ENABLE;
    while (dev->SYNCBUSY.reg) {}
}

void pdm_deinit(void)
{
    SercomSpi *dev = _dev();

    pdm_stop();
    NVIC_DisableIRQ((IRQn_Type)(SERCOM0_0_IRQn + 4 * sercom_id(dev)));
    sercom_clk_dis(dev);
    gpio_disable_mux(pdm_config.clk_pin);
    gpio_disable_mux(pdm_config.data_pin);
}
