/*
 * SPDX-FileCopyrightText: 2025 TU Dresden
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @ingroup     sys_shell_commands
 * @{
 *
 * @file
 * @brief       Shell command implementation for OpenThread
 *
 * The command line is run by the OpenThread CLI in the OpenThread thread:
 * the shell waits until it's done, at most @ref OT_CMD_TIMEOUT_MS. After a
 * timeout, the command runs when the OpenThread thread is free again, the
 * next ones are refused until then.
 *
 * @author      Moritz Voigt <moritz.voigt@mailbox.tu-dresden.de>
 *
 * @}
 */
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "event.h"
#include "irq.h"
#include "mutex.h"
#include "shell.h"
#include "time_units.h"
#include "ztimer.h"
#include "openthread/cli.h"
#include "ot.h"

#ifndef OT_CMD_TIMEOUT_MS
/**
 * @brief   Time given to the OpenThread thread to run a command line
 */
#define OT_CMD_TIMEOUT_MS   (5U * MS_PER_SEC)
#endif

/* copy of the command line: the OpenThread thread can still use it after a
 * timeout, when the line buffer of the shell has the next command */
static char _line[SHELL_DEFAULT_BUFSIZE];
static mutex_t _done = MUTEX_INIT_LOCKED;
/* the last command runs in the OpenThread thread after a timeout */
static bool _busy;

static int ot_console_cb(void *context, const char *format, va_list args)
{
    (void)context;
    return vprintf(format, args);
}

static void _input_handler(event_t *event)
{
    (void)event;
    otCliInputLine(_line);
    mutex_unlock(&_done);
}

static event_t _ev_input = { .handler = _input_handler };

static int ot_cmd(int argc, char **argv)
{
    event_queue_t *queue = openthread_get_evq();
    size_t len = 0;

    if (_busy) {
        /* the command of the timeout returned since */
        if (!mutex_trylock(&_done)) {
            puts("ot: OpenThread still runs the previous command");
            return 1;
        }
        _busy = false;
    }

    /* the arguments after "ot", separated by spaces */
    for (int i = 1; i < argc; i++) {
        int res = snprintf(&_line[len], sizeof(_line) - len, "%s%s", (i > 1) ? " " : "", argv[i]);

        if ((res < 0) || ((size_t)res >= sizeof(_line) - len)) {
            puts("ot: command line too long");
            return 1;
        }
        len += res;
    }
    if (len == 0) {
        strcpy(_line, "help");
    }

    event_post(queue, &_ev_input);
    if (ztimer_mutex_lock_timeout(ZTIMER_MSEC, &_done, OT_CMD_TIMEOUT_MS) < 0) {
        unsigned state = irq_disable();
        bool queued = event_is_queued(queue, &_ev_input);

        event_cancel(queue, &_ev_input);
        irq_restore(state);
        /* not queued: it runs, it unlocks _done when it returns */
        _busy = !queued;
        puts("ot: no answer of the OpenThread thread");
        return 1;
    }
    return 0;
}

SHELL_COMMAND(ot, "Use commands from OpenThread CLI", ot_cmd);

void ot_shell_init(otInstance *aInstance)
{
    otCliInit(aInstance, ot_console_cb, NULL);
}
