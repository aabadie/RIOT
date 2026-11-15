/*
 * SPDX-FileCopyrightText: 2026 Inria
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @ingroup     tests
 * @{
 *
 * @file
 * @brief       Test application for the Adafruit thermal printer driver
 *
 * @author      Alexandre Abadie <alexandre.abadie@inria.fr>
 *
 * @}
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "adafruit_thermal.h"
#include "adafruit_thermal_params.h"
#include "shell.h"

/* side of the squares of the checkerboard, in dots */
#define SQUARE          (16U)
#define PATTERN_HEIGHT  (128U)

static adafruit_thermal_t _dev;

static int _cmd_print(int argc, char **argv)
{
    for (int i = 1; i < argc; i++) {
        adafruit_thermal_print(&_dev, argv[i]);
        adafruit_thermal_print(&_dev, (i == argc - 1) ? "\n" : " ");
    }
    return 0;
}

static int _cmd_feed(int argc, char **argv)
{
    int lines = (argc > 1) ? atoi(argv[1]) : 1;

    if ((lines < 0) || (lines > 255)) {
        printf("usage: %s [lines, 0 to 255]\n", argv[0]);
        return 1;
    }
    adafruit_thermal_feed(&_dev, lines);
    return 0;
}

static int _cmd_mode(int argc, char **argv)
{
    uint8_t mode = ADAFRUIT_THERMAL_MODE_NORMAL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "bold") == 0) {
            mode |= ADAFRUIT_THERMAL_MODE_BOLD;
        }
        else if (strcmp(argv[i], "high") == 0) {
            mode |= ADAFRUIT_THERMAL_MODE_DOUBLE_HEIGHT;
        }
        else if (strcmp(argv[i], "wide") == 0) {
            mode |= ADAFRUIT_THERMAL_MODE_DOUBLE_WIDTH;
        }
        else if (strcmp(argv[i], "strike") == 0) {
            mode |= ADAFRUIT_THERMAL_MODE_STRIKE;
        }
        else if (strcmp(argv[i], "flip") == 0) {
            mode |= ADAFRUIT_THERMAL_MODE_UPSIDE_DOWN;
        }
        else {
            printf("usage: %s [bold] [high] [wide] [strike] [flip]\n", argv[0]);
            return 1;
        }
    }
    adafruit_thermal_set_mode(&_dev, mode);
    return 0;
}

static int _cmd_justify(int argc, char **argv)
{
    if ((argc == 2) && (strcmp(argv[1], "left") == 0)) {
        adafruit_thermal_justify(&_dev, ADAFRUIT_THERMAL_LEFT);
    }
    else if ((argc == 2) && (strcmp(argv[1], "center") == 0)) {
        adafruit_thermal_justify(&_dev, ADAFRUIT_THERMAL_CENTER);
    }
    else if ((argc == 2) && (strcmp(argv[1], "right") == 0)) {
        adafruit_thermal_justify(&_dev, ADAFRUIT_THERMAL_RIGHT);
    }
    else {
        printf("usage: %s <left|center|right>\n", argv[0]);
        return 1;
    }
    return 0;
}

static void _checkerboard(void *arg, unsigned y, uint8_t *row, size_t len)
{
    (void)arg;
    for (unsigned x = 0; x < len * 8; x++) {
        if (((x / SQUARE) + (y / SQUARE)) % 2) {
            row[x / 8] |= 0x80 >> (x % 8);
        }
    }
}

static int _cmd_pattern(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    adafruit_thermal_bitmap(&_dev, ADAFRUIT_THERMAL_WIDTH, PATTERN_HEIGHT,
                            _checkerboard, NULL);
    return 0;
}

static int _cmd_testpage(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    adafruit_thermal_test_page(&_dev);
    return 0;
}

static int _cmd_reset(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    adafruit_thermal_reset(&_dev);
    return 0;
}

static int _cmd_sleep(int argc, char **argv)
{
    long seconds = (argc > 1) ? atol(argv[1]) : 1;

    if ((seconds < 1) || (seconds > (long)UINT16_MAX)) {
        printf("usage: %s [seconds, at least 1]\n", argv[0]);
        return 1;
    }
    adafruit_thermal_sleep(&_dev, seconds);
    return 0;
}

static int _cmd_wake(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    adafruit_thermal_wake(&_dev);
    return 0;
}

static const shell_command_t _commands[] = {
    { "print", "print a line of text", _cmd_print },
    { "feed", "feed the paper [lines]", _cmd_feed },
    { "mode", "set the font [bold] [high] [wide] [strike] [flip]", _cmd_mode },
    { "justify", "align the text <left|center|right>", _cmd_justify },
    { "pattern", "print a checkerboard image", _cmd_pattern },
    { "testpage", "print the test page of the printer", _cmd_testpage },
    { "reset", "reset the printer", _cmd_reset },
    { "sleep", "sleep after [seconds]", _cmd_sleep },
    { "wake", "wake the printer up", _cmd_wake },
    { NULL, NULL, NULL }
};

int main(void)
{
    puts("Adafruit thermal printer test application");

    int res = adafruit_thermal_init(&_dev, &adafruit_thermal_params[0]);
    if (res < 0) {
        printf("init failed: %d\n", res);
        return 1;
    }
    puts("printer ready");

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(_commands, line_buf, SHELL_DEFAULT_BUFSIZE);
    return 0;
}
