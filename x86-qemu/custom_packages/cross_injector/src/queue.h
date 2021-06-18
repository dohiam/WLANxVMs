/*!
 * @file src/queue.h
 * @brief Defines a queue of items.
 * @details
 * This is a simple FIFO queue used by the send, receive, capture, and injection queue looper functions. Note that this is not thread safe.
 * The looper mechanism automaticaly guards the queue for the looper functions.
 *
 * fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).
 */

#ifndef QUEUE_H
#define QUEUE_H

#include "item.h"

struct queue_t {
    struct item_t *current_item;
    struct item_t *last_item;
};

void q_init(struct queue_t * q);
void q_put_item(struct item_t * new_item, struct queue_t *q);                   /* manually add an item to the end of a queue */
struct item_t * q_peek_item(struct queue_t *q);                                 /* get an item at the beginning of a queue without removing it */
struct item_t * q_get_item(struct queue_t *q);                                  /* remove and get an item at the beginning of a queue */
void q_move_item(struct queue_t *q1, struct queue_t *q2);                       /* manually move an item from the beginning of one queue to the end of another */
int q_size(struct queue_t *q);                                                  /* get the size of a queue */
void q_clear(struct queue_t *q);                                                /* flush/free all remaining items on a queue */

#endif // QUEUE_H
