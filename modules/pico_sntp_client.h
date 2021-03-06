/*********************************************************************
    PicoTCP. Copyright (c) 2012 TASS Belgium NV. Some rights reserved.
    See LICENSE and COPYING for usage.

    Author: Toon Stegen
 *********************************************************************/
#ifndef INCLUDE_PICO_SNTP_CLIENT
#define INCLUDE_PICO_SNTP_CLIENT

#include "pico_config.h"
#include "pico_protocol.h"

struct pico_timeval
{
    pico_time tv_sec;
    pico_time tv_msec;
};

int pico_sntp_sync(const char *sntp_server, void (*cb_synced)(pico_err_t status));
int pico_sntp_gettimeofday(struct pico_timeval *tv);

#endif /* _INCLUDE_PICO_SNTP_CLIENT */
