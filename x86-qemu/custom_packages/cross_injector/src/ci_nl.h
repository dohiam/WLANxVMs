/*!
 * @file src/ci_nl.h
 * @brief for using netlink to capture and receive
 * @details
 * This is a work in progress.
 *
 * fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).
 */

#ifndef CI_NL_H
#define CI_NL_H

#define HWSIM_FAMILY_NAME "MAC80211_HWSIM"
#define HWSIM_NL_VERSION 1

#define BUFFER_SIZE 2500
#define NUM_ITEMS 10

#include "queue.h"

int cinl_init(struct queue_t * nlcaptureq);

void cinl_run();

int cinl_received();

#endif // CI_NL_H
