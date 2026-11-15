/*
 * SPDX-FileCopyrightText: 2026 Inria
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @ingroup     drivers_adafruit_thermal
 * @{
 *
 * @file
 * @brief       Driver for the thermal receipt printers sold by Adafruit
 *
 * The commands and the timings come from the Adafruit_Thermal Arduino
 * library.
 *
 * @author      Alexandre Abadie <alexandre.abadie@inria.fr>
 *
 * @}
 */

#include <string.h>

#include "adafruit_thermal.h"
#include "timex.h"
#include "ztimer.h"

#define ASCII_ESC           (0x1bU)
#define ASCII_GS            (0x1dU)
#define ASCII_DC2           (0x12U)

/* the printer reads no command during its boot */
#define BOOT_US             (500U * US_PER_MS)
/* after the wake up byte */
#define WAKE_US             (50U * US_PER_MS)
/* poll period of the DTR pin */
#define DTR_POLL_US         (200U)
/* bytes of image data the printer takes at once */
#define BITMAP_CHUNK_LEN    (256U)
/* the printer stops an image after 255 rows */
#define BITMAP_CHUNK_ROWS   (255U)

#define CHAR_HEIGHT         (24U)
#define CHAR_HEIGHT_DOUBLE  (48U)
#define LINE_SPACING        (6U)
#define MAX_COLUMN          (32U)
#define MAX_COLUMN_DOUBLE   (16U)

static void _wait(adafruit_thermal_t *dev)
{
    if (dev->wait_us) {
        ztimer_sleep(ZTIMER_USEC, dev->wait_us);
        dev->wait_us = 0;
    }
    if (dev->flow_control) {
        /* the printer sets DTR while it is busy */
        while (gpio_read(dev->params.dtr_pin)) {
            ztimer_sleep(ZTIMER_USEC, DTR_POLL_US);
        }
    }
}

/* time the printer takes for what it just received, DTR tells it otherwise */
static void _printing(adafruit_thermal_t *dev, uint32_t us)
{
    if (!dev->flow_control) {
        dev->wait_us = us;
    }
}

static void _send(adafruit_thermal_t *dev, const void *data, size_t len)
{
    _wait(dev);
    uart_write(dev->params.uart, data, len);
}

static void _cmd2(adafruit_thermal_t *dev, uint8_t a, uint8_t b)
{
    uint8_t cmd[] = { a, b };

    _send(dev, cmd, sizeof(cmd));
}

static void _cmd3(adafruit_thermal_t *dev, uint8_t a, uint8_t b, uint8_t c)
{
    uint8_t cmd[] = { a, b, c };

    _send(dev, cmd, sizeof(cmd));
}

static void _cmd4(adafruit_thermal_t *dev, uint8_t a, uint8_t b, uint8_t c,
                  uint8_t d)
{
    uint8_t cmd[] = { a, b, c, d };

    _send(dev, cmd, sizeof(cmd));
}

static void _cmd5(adafruit_thermal_t *dev, uint8_t a, uint8_t b, uint8_t c,
                  uint8_t d, uint8_t e)
{
    uint8_t cmd[] = { a, b, c, d, e };

    _send(dev, cmd, sizeof(cmd));
}

static void _line_start(adafruit_thermal_t *dev)
{
    dev->column = 0;
    dev->prev = '\n';
}

int adafruit_thermal_init(adafruit_thermal_t *dev,
                          const adafruit_thermal_params_t *params)
{
    int res;

    dev->params = *params;
    dev->wait_us = BOOT_US;
    dev->flow_control = false;

    res = uart_init(params->uart, params->baudrate, NULL, NULL);
    if (res != UART_OK) {
        return res;
    }
    if (gpio_is_valid(params->dtr_pin)) {
        res = gpio_init(params->dtr_pin, GPIO_IN_PD);
        if (res < 0) {
            return res;
        }
    }

    adafruit_thermal_wake(dev);
    adafruit_thermal_reset(dev);
    _cmd5(dev, ASCII_ESC, '7', params->heat_dots, params->heat_time,
          params->heat_interval);
    _cmd3(dev, ASCII_DC2, '#',
          (uint8_t)((params->break_time << 5) | (params->density & 0x1f)));
    if (gpio_is_valid(params->dtr_pin)) {
        /* enable the flow control on DTR */
        _cmd3(dev, ASCII_GS, 'a', (1U << 5));
        dev->flow_control = true;
    }

    return 0;
}

void adafruit_thermal_reset(adafruit_thermal_t *dev)
{
    static const uint8_t tabs[] = {
        ASCII_ESC, 'D', 4, 8, 12, 16, 20, 24, 28, 0
    };

    _cmd2(dev, ASCII_ESC, '@');
    _send(dev, tabs, sizeof(tabs));
    _line_start(dev);
    dev->max_column = MAX_COLUMN;
    dev->char_height = CHAR_HEIGHT;
    dev->line_spacing = LINE_SPACING;
}

void adafruit_thermal_write(adafruit_thermal_t *dev, const char *text, size_t len)
{
    for (size_t i = 0; i < len; i++) {
        char c = text[i];

        if (c == '\r') {
            continue;
        }
        _send(dev, &c, 1);
        if ((c == '\n') || (dev->column == dev->max_column)) {
            if (dev->prev == '\n') {
                /* an empty line, only a feed */
                _printing(dev, (dev->char_height + dev->line_spacing)
                          * dev->params.dot_feed_us);
            }
            else {
                _printing(dev, dev->char_height * dev->params.dot_print_us
                          + dev->line_spacing * dev->params.dot_feed_us);
            }
            dev->column = 0;
            /* the printer starts a new line after the last column */
            c = '\n';
        }
        else {
            dev->column++;
        }
        dev->prev = c;
    }
}

void adafruit_thermal_print(adafruit_thermal_t *dev, const char *str)
{
    adafruit_thermal_write(dev, str, strlen(str));
}

void adafruit_thermal_feed(adafruit_thermal_t *dev, uint8_t lines)
{
    _cmd3(dev, ASCII_ESC, 'd', lines);
    _printing(dev, lines * dev->char_height * dev->params.dot_feed_us);
    _line_start(dev);
}

void adafruit_thermal_feed_rows(adafruit_thermal_t *dev, uint8_t rows)
{
    _cmd3(dev, ASCII_ESC, 'J', rows);
    _printing(dev, rows * dev->params.dot_feed_us);
    _line_start(dev);
}

void adafruit_thermal_set_mode(adafruit_thermal_t *dev, uint8_t mode)
{
    _cmd3(dev, ASCII_ESC, '!', mode);
    dev->char_height = (mode & ADAFRUIT_THERMAL_MODE_DOUBLE_HEIGHT)
                       ? CHAR_HEIGHT_DOUBLE : CHAR_HEIGHT;
    dev->max_column = (mode & ADAFRUIT_THERMAL_MODE_DOUBLE_WIDTH)
                      ? MAX_COLUMN_DOUBLE : MAX_COLUMN;
}

void adafruit_thermal_justify(adafruit_thermal_t *dev,
                              adafruit_thermal_justify_t justify)
{
    _cmd3(dev, ASCII_ESC, 'a', justify);
}

void adafruit_thermal_bitmap(adafruit_thermal_t *dev, unsigned width,
                             unsigned height, adafruit_thermal_row_cb_t cb,
                             void *arg)
{
    uint8_t row[ADAFRUIT_THERMAL_ROW_LEN];
    unsigned row_len;
    unsigned chunk_max;

    if (width > ADAFRUIT_THERMAL_WIDTH) {
        width = ADAFRUIT_THERMAL_WIDTH;
    }
    row_len = (width + 7) / 8;
    if (row_len == 0) {
        return;
    }
    chunk_max = BITMAP_CHUNK_LEN / row_len;
    if (chunk_max > BITMAP_CHUNK_ROWS) {
        chunk_max = BITMAP_CHUNK_ROWS;
    }

    for (unsigned y = 0; y < height;) {
        unsigned rows = height - y;
        if (rows > chunk_max) {
            rows = chunk_max;
        }
        _cmd4(dev, ASCII_DC2, '*', rows, row_len);
        for (unsigned i = 0; i < rows; i++, y++) {
            memset(row, 0, row_len);
            cb(arg, y, row, row_len);
            _send(dev, row, row_len);
        }
        _printing(dev, rows * dev->params.dot_print_us);
    }
    _line_start(dev);
}

typedef struct {
    const uint8_t *bitmap;
    size_t stride;
} _buf_t;

static void _buf_row(void *arg, unsigned y, uint8_t *row, size_t len)
{
    const _buf_t *buf = arg;

    memcpy(row, &buf->bitmap[y * buf->stride], len);
}

void adafruit_thermal_bitmap_buf(adafruit_thermal_t *dev, unsigned width,
                                 unsigned height, const uint8_t *bitmap)
{
    /* a wider image prints clipped, its rows keep their length */
    _buf_t buf = { .bitmap = bitmap, .stride = (width + 7) / 8 };

    adafruit_thermal_bitmap(dev, width, height, _buf_row, &buf);
}

void adafruit_thermal_test_page(adafruit_thermal_t *dev)
{
    _cmd2(dev, ASCII_DC2, 'T');
    /* 26 lines of text, and a few feeds */
    _printing(dev, dev->params.dot_print_us * 24 * 26
              + dev->params.dot_feed_us * (6 * 26 + 30));
    _line_start(dev);
}

void adafruit_thermal_sleep(adafruit_thermal_t *dev, uint16_t seconds)
{
    if (seconds == 0) {
        /* 0 disables the sleep */
        seconds = 1;
    }
    _cmd4(dev, ASCII_ESC, '8', seconds & 0xff, seconds >> 8);
}

void adafruit_thermal_wake(adafruit_thermal_t *dev)
{
    uint8_t wake = 0xff;

    _send(dev, &wake, 1);
    dev->wait_us = WAKE_US;
    /* disable the sleep */
    _cmd4(dev, ASCII_ESC, '8', 0, 0);
}
