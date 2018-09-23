/*
 * Copyright (C) 2018 Inria
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     tests
 * @{
 *
 * @file
 * @brief       Test application for RN4870/1 BLE module driver
 *
 * @author      Alexandre Abadie <alexandre.abadie@inria.fr>
 *
 * @}
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "timex.h"
#include "shell.h"

#include "rn487x.h"
#include "rn487x_params.h"
#include "rn487x_internal.h"

static rn487x_t rn487x_dev;
// static uint8_t payload[RN487X_MAX_BUF];

static void _print_usage(void)
{
    puts("Usage: rn487x <sleep|reset|factoryRESET>");
}

int rn487x_cmd(int argc, char **argv) {
    if (argc < 2) {
        _print_usage();
        return -1;
    }

    if (strcmp(argv[1], "factoryRESET") == 0) {
        if (rn487x_factory_reset(&rn487x_dev) != RN487X_OK) {
            puts("Factory reset failed");
            return -1;
        }
    }
    else {
        _print_usage();
        return -1;
    }

    return 0;
}

static const shell_command_t shell_commands[] = {
    { "rn487x",   "Run RN487X commands", rn487x_cmd },
    { NULL, NULL, NULL }
};

int main(void)
{
    puts("RN487X device driver test");

    rn487x_setup(&rn487x_dev, &rn487x_params[0]);
    if (rn487x_init(&rn487x_dev) != RN487X_OK) {
        puts("RN487X initialization failed");
        return -1;
    }

    /* start the shell */
    puts("Initialization OK, starting shell now");

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
