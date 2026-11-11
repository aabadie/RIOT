/*
 * SPDX-FileCopyrightText: 2026 Inria
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @file
 * @brief       OpenThread border router example
 *
 * The border router of the Thread network is in the OpenThread package
 * (openthread_border_router): the prefix of the host is published in the
 * Thread network, the IPv6 datagrams are bridged between the Thread network
 * and the host over ethos. The shell (ot command) is on ethos too.
 *
 * @author      Alexandre Abadie <alexandre.abadie@inria.fr>
 */

#include <stdio.h>

#include "shell.h"

int main(void)
{
    puts("OpenThread border router example");

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
