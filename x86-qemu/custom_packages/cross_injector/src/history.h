/*!
 * @file src/history.h
 * @brief can be used to ensure the same packet is not injected more than once
 * @details
 * The motivation for this is to avoid an injected packet to be captured and injected back, creating a storm. In practice, this does not
 * appear to happen, so this does not currently seem to be needed.
 *
 * fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).
 */

#ifndef HISTORY_H
#define HISTORY_H

#include <stdint.h>
#include "pcap.h"

#define HISTORY 100
#define HASH_MULTIPLIER 257

void history_init();

uint64_t history_hash(const u_char *pkt,uint32_t pkt_size);

int history_was_injected(uint8_t * pkt, uint32_t pkt_size);

#ifdef HISTORY_TEST
void history_test();
#endif

#endif
