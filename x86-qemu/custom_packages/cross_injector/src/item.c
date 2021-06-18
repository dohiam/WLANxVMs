/*!
 * @file src/item.c
 * @brief implements the "item" class
 * @details
 * Two memory management approaches can be used. The first is regular malloc. Since every time an packet is received or captured, memory for a new
 * item and its buffer are needed, a second approach using a fixed size memory buffer can be used to try to increase performance (at the expense
 * of allocating a large amount of memory up front). In practice this does not seem to increase performance significantly.
 *
 * fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).
 */

#include "item.h"
#include "history.h"
#include <stdlib.h>
#include <string.h>

#ifdef USE_FS_MALLOC
#include "utilities.h"
#define malloc(x) fs_malloc(x);
#define free(x) fs_free(x);
#endif

struct item_t * item_make(uint8_t * buffer, uint32_t size) {
    struct item_t *new_item;
    new_item = malloc(sizeof(struct item_t));
    if (!new_item) return NULL;
    new_item->buffer = malloc(size);
    if (!new_item->buffer) {
        free(new_item);
        return NULL;
    }
    memcpy(new_item->buffer, buffer, size);
    new_item->size = size;
    new_item->next_item = NULL;
    return new_item;
}

 void item_free(struct item_t ** dipp) {
    if (*dipp) {
        if ((*dipp)->buffer) free((*dipp)->buffer);
        free(*dipp);
    }
    *dipp = NULL;
 }

int item_match(struct item_t *item1, struct item_t * item2) {
    if (!item1 || !item2 || !item1->buffer || !item2->buffer) return 0;
    if (item1->size != item2->size) return 0;
    for (int i=0; i<item1->size; i++) if (item1->buffer[i] != item2->buffer[i]) return 0;
    return 1;
}

void item_copy(struct item_t *item1, struct item_t ** item2) {
    *item2 = item_make(item1->buffer, item1->size);
}


