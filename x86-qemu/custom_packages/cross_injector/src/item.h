/*!
 * @file src/item.h
 * @brief defines the "item" class
 * @details
 * All packets are considered to be instances of a more generic "item" which can be created, moved, destroyed, etc.
 *
 * Rationale:
 * The core processing (capture, send, receive, inject, move) is not specific to Wi-Fi packets. They could be used for other
 * types of packets, or even things that aren't packets.
 *
 * fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).
 */

#ifndef ITEMS_H
#define ITEMS_H

//#define USE_FS_MALLOC 1

#include <stdint.h>

struct item_t {
    uint32_t size;
    uint8_t * buffer;
    struct item_t *next_item;
};

struct item_t * item_make(uint8_t * buffer, uint32_t size);

void item_free(struct item_t ** dipp);

int item_match(struct item_t *item1, struct item_t * item2);

void item_copy(struct item_t *item1, struct item_t ** item2);

#endif // ITEMS

