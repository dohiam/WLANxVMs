/*
 queue using a fixed size array that producter and consumer worker threads cycle through
 intended use: add items to the queue in one thread and use them in another
 restruction: get one item and finish using it before getting another and also finish putting an item before putting another
 i.e., head_used and tail_done are what actually advance the head and tail pointers, get_head and get_tail do not
 e.g., thread A: get_head(), head_used(), get_head(), head_used(), ...
       thread B: get_tail(), tail_done(), get_tail(), tail_done(), ...

*/
#ifndef QUEUE_H
#define QUEUE_H

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

typedef struct aq_struct {
    uint16_t head;
    uint16_t tail;
    uint16_t item_size;
    uint16_t num_items;
    pthread_mutex_t lock;
    void    *data;
    bool    full;
    bool    empty;
}aq_type;

// allocates and initializes q struct/data
bool aq_new(aq_type ** q, uint16_t item_size, uint16_t num_items);

// gets pointer to head data, if any
void * aq_get_head(aq_type * q);

// gets pointer to tail data, if q is not full
void * aq_get_tail(aq_type * q);

// must call this when done using auired head for it to be able to be reused
void aq_used_head(aq_type * q);

// must call this when finished filling in tail data before getting the next tail
void aq_put_tail(aq_type * q);

void aq_free(aq_type *q);


#endif
