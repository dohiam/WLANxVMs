/*!
 * @file src/queue.c
 * @brief Implements a queue of items.
 *
 * fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).
 */

#include "queue.h"
#include <unistd.h>
#include <stdio.h>

static int q_initialized = 0;

void q_init(struct queue_t * q) {
    q_initialized = 1;
    q->current_item = q->last_item = NULL;
}


void q_put_item(struct item_t * new_item, struct queue_t *q) {
    if (!q_initialized) {
        printf("error: did not initialize queue (q_init)\n");
        return;
    }
    if (!q->current_item) q->current_item = q->last_item = new_item;
    else {
            q->last_item->next_item = new_item;
            q->last_item = new_item;
            q->last_item->next_item = NULL;
    }
}

struct item_t * q_peek_item(struct queue_t *q) {
    if (!q_initialized) {
        printf("error: did not initialize queue (q_init)\n");
        return NULL;
    }
    return q->current_item;
}

struct item_t * q_get_item(struct queue_t *q) {
    struct item_t * item;
    if (!q_initialized) {
        printf("error: did not initialize queue (q_init)\n");
        return NULL;
    }
    item = q->current_item;
    if (q->current_item) q->current_item=q->current_item->next_item;
    else q->last_item = NULL;
    if (item) item->next_item = NULL;
    return item;
}

void q_move_item(struct queue_t *q1, struct queue_t *q2) {
    struct item_t *item;
    if (!q_initialized) {
        printf("error: did not initialize queue (q_init)\n");
        return;
    }
    item = q_get_item(q1);
    q_put_item(item, q2);
}

int q_size(struct queue_t *q) {
    int n;
    struct item_t * item;
    if (!q_initialized) {
        printf("error: did not initialize queue (q_init)\n");
        return 0;
    }
    item = q->current_item;
    n = 0;
    while (item) {
        n++;
        item=item->next_item;
    }
    return n;
}

void q_clear(struct queue_t *q) {
     struct item_t *item, *next_item;
    if (!q_initialized) {
        printf("error: did not initialize queue (q_init)\n");
        return;
    }
     item = q->current_item;
     while (item) {
        next_item = item->next_item;
        item_free(&item);
        item = next_item;
     }
     q->current_item = q->last_item = NULL;
}
