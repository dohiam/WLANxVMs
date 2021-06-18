
#include "queue.h"
#include <stdlib.h>
/*
 * initially, head==tail and q is empty
 * when using an item from the q, head increments and wraps around
 * when putting an item to the q, tail increments and wraps around
 * if using an item from the q, head meets tail then q is empty and can get no more items
 * if putting an item to the q, tail meets head then q is full and can put no more items
 */

// allocates and initializes q struct/data
bool aq_new(aq_type ** q, uint16_t item_size, uint16_t num_items) {
    *q = malloc(sizeof(aq_type));
    if (!*q) return false;
    (*q)->data = malloc(item_size * num_items);
    if (!(*q)->data) {
        free(q);
        *q = NULL;
        return false;
    }
    (*q)->head = (*q)->tail = 0;
    (*q)->item_size = item_size;
    (*q)->num_items = num_items;
    (*q)->full = false;
    (*q)->empty = true;
    pthread_mutex_init(&((*q)->lock), NULL);
    return true;
}

void * aq_head(aq_type * q) {
    return q->data + (q->item_size * q->head);
}

void * aq_tail(aq_type * q) {
    return q->data + (q->item_size * q->tail);
}

// gets pointer to head data unless q is empty
void * aq_get_head(aq_type * q) {
    void *ret = NULL;
    pthread_mutex_lock(&(q->lock));
    if (!q->empty) {
        ret = aq_head(q);
    }
    pthread_mutex_unlock(&(q->lock));
    return ret;
}

// gets pointer to tail unless q is full
void * aq_get_tail(aq_type * q) {
    void *ret = NULL;
    pthread_mutex_lock(&(q->lock));
    if (!q->full) {
        ret = aq_tail(q);
    }
    pthread_mutex_unlock(&(q->lock));
    return ret;
}

// indicates that memory associated with head has been consumed and can be reused
void aq_used_head(aq_type * q) {
    pthread_mutex_lock(&(q->lock));
    if (++(q->head) == q->num_items) q->head = 0;
    if (q->head == q->tail) {
        q->empty = true;
    }
    q->full = false;
    pthread_mutex_unlock(&(q->lock));
}

// indicates that memory associated with tail has been fully produced
void aq_put_tail(aq_type * q) {
    pthread_mutex_lock(&(q->lock));
    if (++(q->tail) == q->num_items) q->tail = 0;
    if (q->tail == q->head) {
        q->full = true;
    }
    q->empty = false;
    pthread_mutex_unlock(&(q->lock));
}

void aq_free(aq_type *q) {
    free(q->data);
    free(q);
}

