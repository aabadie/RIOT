/*
 * SPDX-FileCopyrightText: 2026 Inria
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @file
 * @brief       OpenThread Minimal Thread Device example
 *
 * The node joins the Thread network of the OPENTHREAD_* settings at boot as
 * the child of a router, a sleepy end device when POLL_PERIOD_MS is not 0,
 * prints its role when it changes and serves the name of its board over CoAP
 * (riot/board). The OpenThread CLI is the ot shell command.
 *
 * @author      Alexandre Abadie <alexandre.abadie@inria.fr>
 */

#include <stdio.h>
#include <string.h>

#include "event.h"
#include "shell.h"

#include "ot.h"
#include "openthread/coap.h"
#include "openthread/link.h"
#include "openthread/thread.h"

#ifndef POLL_PERIOD_MS
#define POLL_PERIOD_MS  (1000U)
#endif

static void _board_handler(void *context, otMessage *request, const otMessageInfo *info)
{
    (void)context;

    otInstance *ot = openthread_get_instance();
    otCoapType type = (otCoapMessageGetType(request) == OT_COAP_TYPE_CONFIRMABLE)
                      ? OT_COAP_TYPE_ACKNOWLEDGMENT : OT_COAP_TYPE_NON_CONFIRMABLE;
    otCoapCode code = (otCoapMessageGetCode(request) == OT_COAP_CODE_GET)
                      ? OT_COAP_CODE_CONTENT : OT_COAP_CODE_METHOD_NOT_ALLOWED;
    otMessage *response = otCoapNewMessage(ot, NULL);

    if (response == NULL) {
        return;
    }
    otCoapMessageInitResponse(response, request, type, code);
    if (code == OT_COAP_CODE_CONTENT) {
        otCoapMessageSetPayloadMarker(response);
        otMessageAppend(response, RIOT_BOARD, strlen(RIOT_BOARD));
    }
    if (otCoapSendResponse(ot, response, info) != OT_ERROR_NONE) {
        otMessageFree(response);
    }
}

static otCoapResource _board_resource = {
    .mUriPath = "riot/board",
    .mHandler = _board_handler,
};

static void _state_changed(otChangedFlags flags, void *context)
{
    (void)context;

    if (flags & OT_CHANGED_THREAD_ROLE) {
        otDeviceRole role = otThreadGetDeviceRole(openthread_get_instance());

        printf("openthread: %s\n", otThreadDeviceRoleToString(role));
    }
}

/* the OpenThread API is only called from the OpenThread thread, in the
 * handlers of the events of its queue */
static void _setup(event_t *event)
{
    (void)event;

    otInstance *ot = openthread_get_instance();

    if (POLL_PERIOD_MS > 0) {
        /* sleepy end device: no reception when idle, the parent keeps the
         * frames for the child until its next poll */
        otLinkModeConfig mode = { .mRxOnWhenIdle = false };

        otLinkSetPollPeriod(ot, POLL_PERIOD_MS);
        otThreadSetLinkMode(ot, mode);
        printf("openthread: sleepy end device, poll period %u ms\n",
               (unsigned)POLL_PERIOD_MS);
    }

    otSetStateChangedCallback(ot, _state_changed, NULL);
    otCoapStart(ot, OT_DEFAULT_COAP_PORT);
    otCoapAddResource(ot, &_board_resource);
    printf("openthread: CoAP server on port %u\n", OT_DEFAULT_COAP_PORT);
}

static event_t _ev_setup = { .handler = _setup };

int main(void)
{
    puts("OpenThread MTD example");

    event_post(openthread_get_evq(), &_ev_setup);

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
