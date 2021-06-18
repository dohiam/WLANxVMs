/*!
 * @file src/ci_queues.h
 * @brief defines the sending, receiving, capturing, and injecting queues.
 * @details
 * The client part of this is the only part needed for production. The server is used for testing (to test the client).
 *
 *  Keeping the following picture in mind should help:
 *
 *      . USING NETWORKING
 *      .
 *      .                   capture        /--------\   send       receive    /--------\         inject
 *      .   |------|  ----------------->  |          |  ----------------->   |          |  ----------------->   |-------|
 *      .   |  AP  | <------------------  |  server  |  <-----------------   |  client  |  <-----------------   |  STA  |
 *      .   |------|        inject        |          |  receive       send   |          |        capture        |-------|
 *      .                                  \--------/                         \--------/
 *      .
 *
 *      . USING VSOCK:
 *      .
 *      .                   capture        /--------\   send         receive   /--------\   send       receive    /--------\         inject
 *      .   |------|  ----------------->  |          |    ----------------->  |   host   |  ----------------->   |          |  ----------------->   |-------|
 *      .   |  AP  | <------------------  |  client  |   <------------------  |  server  |  <-----------------   |  client  |  <-----------------   |  STA  |
 *      .   |------|        inject        |          |  receive         send  |          |  receive       send   |          |        capture        |-------|
 *      .                                  \--------/                          \--------/                         \--------/
 *      .
 *
 * There is a queue for each arrow, and a thread for each queue.
 * Capture and inject are via hwsim0.
 * Client and server each have both a send and receive queue to share packets. The send and receive queue together is referred to as "sharing".
 *
 * fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).
 */

#ifndef QUEUES_H
#define QUEUES_H

#include <stdint.h>
#include "item.h"
#include "lisa.h"
#include <pcap.h>

#define Q_SOCKET_ERROR -1
#define Q_PCAP_ERROR -2
#define Q_NO_ERROR 0

#define PCAP_DEVICE "hwsim0"

#define BUFFER_SIZE 2500
#define QUEUE_SIZE 10

enum ciqs_queue {sendq, altsendq, receiveq, altreceiveq, captureq, nlcaptureq, injectionq, nlinjectionq, naq};

#define CIQS_FIRST_QUEUE sendq
#define CIQS_LAST_QUEUE  nlinjectionq+1
#define CIQS_NUM_QUEUES  CIQS_LAST_QUEUE

void ciqs_init_queues(); /* call before doing anything with queues */

// for send, altsend, receive, altreceive, dataptr is a pointer to a lisa
// for capture and injection, dataptr is a pointer to a pcap_handle
void ciqs_start(enum ciqs_queue q, void * dataptr);
void ciqs_stop(enum ciqs_queue q);
void ciqs_pause(enum ciqs_queue q);
void ciqs_resume(enum ciqs_queue q);

void ciqs_stop_all();

void ciqs_put_item_ts(struct item_t * new_item, enum ciqs_queue q);
struct item_t * ciqs_get_item_ts(enum ciqs_queue q);
struct item_t * ciqs_peek_item_ts(enum ciqs_queue q);
int ciqs_size_ts(enum ciqs_queue q);

void ciqs_print_stats();

#endif
