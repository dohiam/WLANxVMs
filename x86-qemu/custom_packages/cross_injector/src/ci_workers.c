/*!
 * @file src/ci_workers.c
 * @brief implements the workers to move packets from capture to send and from receive to inject, and the listen socket for a server
 * @details
 * Each worker is implemented as a looper (/ref src/looper.h).
 * The implementation allows any number of workers to be run moving things between any queue to any other queue, but for the
 * cross_injector, only moving from capture to send and from receive to inject are used.
 *
 * fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).
 */

#include "ci_workers.h"
#include "ci_main.h"
#include "ci_queues.h"
#include "looper.h"
#include "item.h"
#include "utilities.h"
#include "debug.h"

static struct looper_t listener;

struct listening_data_t {
    lisa *lp;
    int  num_clients;
};

static struct listening_data_t listening_data;

static void repeat_listening_function(void *d);

void start_listener(lisa *lp) {
    looper_init(&listener);
    listening_data.lp = lp;
    listening_data.num_clients = 0;
    listener.function_to_run_first = NULL;
    listener.function_to_run_last = NULL;
    listener.function_to_repeat = repeat_listening_function;
    looper_start(&listener);
}


/**************************************************************************************************************************
 *
 *  Thread Functions
 *
 **************************************************************************************************************************/

static void repeat_listening_function(void *d) {
    struct listening_data_t * ld = (struct listening_data_t *) d;
    lisa_accept(ld->lp, ld->lp->port);
    if (ld->lp->num_clients > ld->num_clients) {
        printf("received connection\n");
        ld->num_clients = ld->lp->num_clients;
    }
}

