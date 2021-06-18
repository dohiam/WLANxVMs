/*!
 * @file src/ci_queues.c
 * @brief implements the sending, receiving, capturing, and injecting queues.
 * @details
 * Each queue is implemented as a looper (/ref src/looper.h) along with its own queue data.
 * used as a normal IP address.
 *
 * fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).
 */

#include "ci_queues.h"
#include "aq_type.h"
#include "ci_main.h"
#include "item.h"
#include "looper.h"
#include "utilities.h"
#include "ci_nl.h"
#include "radiotap.h"
#include "debug.h"
#include "bignum_sec_profiling.h"
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <inttypes.h>


/***********************************************************************
 * stats
 ***********************************************************************/

static int packets_received=0;
static int packets_sent=0;
static int packets_captured=0;
static int packets_injected=0;
static int packets_nlinjected=0;

void ciqs_print_stats() {
    bignum_sec_t avg;
    bignum_sec_t max;
    bignum_sec_t min;
    printf("\n");
    printf("packets_captured: %d\n", packets_captured);
    printf("packets_sent:     %d\n", packets_sent);
    printf("packets_received: %d\n", packets_received);
    printf("packets_injected: %d\n", packets_injected);
    printf("packets_nlinjected: %d\n", packets_nlinjected);
    profiling_get_lag_time_avg(&avg, packets_injected);
    profiling_get_lag_time_max(&max);
    profiling_get_lag_time_min(&min);
    if (avg.positive) printf("avg lag time: +%010ld.%09ld \n", avg.sec, avg.nsec);
    else printf("avg lag time: -%010ld.%09ld \n", avg.sec, avg.nsec);
    if (max.positive) printf("max lag time: +%010ld.%09ld \n", max.sec, max.nsec);
    else printf("max lag time: -%010ld.%09ld \n", max.sec, max.nsec);
    if (min.positive) printf("min lag time: +%010ld.%09ld \n", min.sec, min.nsec);
    else printf("min lag time: -%010ld.%09ld \n", min.sec, min.nsec);
}

/***********************************************************************
 * queue data
 ***********************************************************************/

static char pcap_errbuf[PCAP_ERRBUF_SIZE]; /* Size defined in pcap.h */

static aq_type * capture_send_q;     /* capture is the producer, send is the consumer */
static aq_type * receive_inject_q;   /* receive is the producer, inject is the consumer */

typedef struct q_item_s {
    uint32_t size;
    bignum_sec_t bignum_timestamp;
    uint8_t  buffer[BUFFER_SIZE];
} q_item_t;

struct sending_data_t {
    lisa *lp;
    uint8_t len[4];
    uint8_t timestamp[16];
    uint32_t len_processed;
    uint32_t pkt_processed;
    int in_process;
    q_item_t * item;
};

struct receiving_data_t {
    lisa *lp;
    uint8_t len[4];
    uint8_t timestamp[16];
    uint32_t bytes_received;
    uint32_t bytes_to_receive;
    int len_received;
    q_item_t * item;
};

struct pcap_data_t {
    pcap_t **pcap_handle_ptr;
    char * dev;
};

union ciqs_data_t {
    struct sending_data_t send_data;         /* for sending only */
    struct receiving_data_t receive_data;    /* for receiving only */
    struct pcap_data_t pcap_data;            /* for capture and inject */
};

static struct ciqs_t {
    struct looper_t looper;
    union ciqs_data_t data;
} ciqs[CIQS_NUM_QUEUES];

/***********************************************************************
 * queue functions
 ***********************************************************************/

#define NUM_QUEUES 4

static enum ciqs_queue queues_running[NUM_QUEUES];
static int last_queue;

static void add_running_queue(enum ciqs_queue q) {
    queues_running[last_queue++] = q;
}

static void del_running_queue(enum ciqs_queue q) {
    int i, found;
    found = 0;
    for (i=0; i<NUM_QUEUES && !found; i++) {
        if (queues_running[i] == q) {
            found = 1;
            for (int j=i+1; j<NUM_QUEUES; j++) queues_running[j-1] = queues_running[j];
            queues_running[last_queue] = naq;
            last_queue--;
        }
    }
}

void ciqs_stop_all() {
    for (int i=0; i<NUM_QUEUES; i++) if (queues_running[i] != naq) ciqs_stop(queues_running[i]);
}

static void repeat_sending_function(void *d);
static void repeat_receiving_function(void *d);
static void repeat_capturing_function(void *d);
static void repeat_injecting_function(void *d);
static void repeat_nlcapturing_function(void *d);
static void repeat_nlinjecting_function(void *d);

static void (* looper_functions[CIQS_NUM_QUEUES]) (void *d) = {repeat_sending_function, repeat_sending_function,
                                                               repeat_receiving_function, repeat_receiving_function,
                                                               repeat_capturing_function, repeat_nlcapturing_function,
                                                               repeat_injecting_function, repeat_nlinjecting_function};

static void init_sending_function(void *d);
static void init_receiving_function(void *d);
static void init_capturing_function(void *d);
static void init_injecting_function(void *d);
static void init_nlcapturing_function(void *d);
static void init_nlinjecting_function(void *d);

static void (*init_functions[CIQS_NUM_QUEUES]) (void *d) = {init_sending_function, init_sending_function,
                                                            init_receiving_function, init_receiving_function,
                                                            init_capturing_function, init_nlcapturing_function,
                                                            init_injecting_function, init_nlinjecting_function};

static void common_final_function(void *d);

void ciqs_init_queues() {
    if (!aq_new(&capture_send_q, sizeof(q_item_t), QUEUE_SIZE)) {
        printf("error allocating capture->send queue\n");
        return;
    }
    if (!aq_new(&receive_inject_q, sizeof(q_item_t), QUEUE_SIZE)) {
        printf("error allocating receive->inject queue\n");
        return;
    }
    for (enum ciqs_queue q = CIQS_FIRST_QUEUE; q<CIQS_LAST_QUEUE; q++) {
            looper_init(&(ciqs[q].looper));
            ciqs[q].looper.function_to_run_first = init_functions[q];
            ciqs[q].looper.function_to_repeat = looper_functions[q];
            ciqs[q].looper.function_to_run_last = common_final_function;
            ciqs[q].looper.data = (void *) &(ciqs[q]);
    }
    for (int i=0; i<NUM_QUEUES; i++) queues_running[i] = naq;
    last_queue = 0;
}

void ciqs_start(enum ciqs_queue q, void * dataptr) {
    switch (q) {
        case sendq:
            ciqs[sendq].data.send_data.lp = (lisa *) dataptr;
            looper_start(&(ciqs[sendq].looper));
            break;
        case altsendq:
            ciqs[altsendq].data.send_data.lp = (lisa *) dataptr;
            looper_start(&(ciqs[altsendq].looper));
            break;
        case receiveq:
            ciqs[receiveq].data.receive_data.lp = (lisa *) dataptr;
            looper_start(&(ciqs[receiveq].looper));
            break;
        case altreceiveq:
            ciqs[altreceiveq].data.receive_data.lp = (lisa *) dataptr;
            looper_start(&(ciqs[altreceiveq].looper));
            break;
        case captureq:
            ciqs[captureq].data.pcap_data.pcap_handle_ptr = (pcap_t **) dataptr;
            looper_nowait(&(ciqs[captureq].looper));
            looper_start(&(ciqs[captureq].looper));
            break;
        case injectionq:
            ciqs[injectionq].data.pcap_data.pcap_handle_ptr = (pcap_t **) dataptr;
            looper_start(&(ciqs[injectionq].looper));
            break;
        case nlcaptureq:
            looper_start(&(ciqs[nlcaptureq].looper));
            break;
        case nlinjectionq:
            looper_start(&(ciqs[nlinjectionq].looper));
            break;
        case naq: break;
    }
    add_running_queue(q);
}

void ciqs_stop (enum ciqs_queue q)  { looper_stop(&(ciqs[q].looper)); del_running_queue(q); }
void ciqs_pause(enum ciqs_queue q)  { looper_pause(&(ciqs[q].looper)); }
void ciqs_resume(enum ciqs_queue q) { looper_resume(&(ciqs[q].looper)); }


/**************************************************************************************************************************
 *
 *  Thread Functions
 *
 **************************************************************************************************************************/

 /***************
  * sending
  ***************/

void common_final_function(void * d) {

}

 /***************
  * sending
  ***************/

// sets the length of the packet from send q worker to receive q worker in the other VM, big endian
static void set_length(uint8_t len[4], uint32_t size) {
    len[3] = (uint8_t) size & 0x000000FF;
    size >>= 8;
    len[2] = (uint8_t) size & 0x000000FF;
    size >>= 8;
    len[1] = (uint8_t) size & 0x000000FF;
    size >>= 8;
    len[0] = (uint8_t) size & 0x000000FF;
}

static void set_timestamp (uint8_t timestamp[16], q_item_t * item){
    /* marshal seconds and then nseconds both in BE format */
    int i;
    uint8_t byte;
    uint64_t byte8;
    byte8 = item->bignum_timestamp.sec;
    for (i=0; i < 8; i++) {
        byte = byte8 & (uint64_t) 0xFF;
        timestamp[7-i] = byte;
        byte8 = byte8 >> 8;
    }
    byte8 = item->bignum_timestamp.nsec;
    for (i=0; i < 8; i++) {
        byte = byte8 & (uint64_t) 0xFF;
        timestamp[15-i] = byte;
        byte8 = byte8 >> 8;
    }
}

static void init_sending_function(void *d) {
    struct ciqs_t * q = (struct ciqs_t *) d;
    struct sending_data_t * sd = (struct sending_data_t *) &(q->data);
    sd->item = NULL;
    sd->len_processed = 0;
    sd->pkt_processed = 0;
    sd->in_process = 0;
}

 static void repeat_sending_function(void *d) {
    bool q2_condition;
    struct ciqs_t * ciqs_ptr = (struct ciqs_t *) d;
    struct sending_data_t *sd = (struct sending_data_t *) &(ciqs_ptr->data.send_data);
    if (!sd->in_process) {
        if (sd->lp->state >= LISA_CONNECTED) sd->item = (q_item_t *) aq_get_head(capture_send_q);
        if (sd->item) {
            #ifdef FORCE_POWERSAVE_OFF
            force_powersave_flag_off(sd->item->buffer, sd->item->size);
            #endif
            sd->in_process = 1;
            q2_set_condition(q2_condition, (!DEBUG || (Q2PRINT_PROCESSING && (Q2PRINT_BEACONS || !is_beacon(sd->item->buffer, sd->item->size)))));
            q2log(q2_condition, "got item to send");
            q2log_wftype(q2_condition, sd->item->buffer, sd->item->size, "to send");
        }
    }
    if (sd->in_process) {
        // as long as have not sent the number of bytes, set the length and just send it
        if (sd->len_processed < 4) {
            set_length(sd->len, sd->item->size);
            sd->len_processed += lisa_cast(sd->lp, (char *) sd->len + sd->len_processed, 4 - sd->len_processed);
            q2log(q2_condition, "send len processed: %d, lp:%p", sd->len_processed, (void *) sd->lp);
        }
        if ( (4 <= sd->len_processed)  && (sd->len_processed < 20) )  {
            set_timestamp(sd->timestamp, sd->item);
            sd->len_processed += lisa_cast(sd->lp, (char *) sd->timestamp + (sd->len_processed - 4), 20  - sd->len_processed);
            q2log(q2_condition, "send timestamp processed:");
            q2log_hex(q2_condition, sd->timestamp, 16);
        }
        // once sent the length bytes, then send the rest of the one packet to send
        if (sd->len_processed == 20) {
            sd->pkt_processed += lisa_cast(sd->lp, (char *) sd->item->buffer + sd->pkt_processed, sd->item->size - sd->pkt_processed);
            assert(sd->pkt_processed <= sd->item->size);
            if (sd->pkt_processed == sd->item->size) {
                q2log(q2_condition, "sent %d, (%d)", sd->item->size, packets_sent++);
                q2log_wftype(q2_condition, sd->item->buffer, sd->item->size, "sent");
                packets_sent++;
                aq_used_head(capture_send_q);
                sd->in_process = 0;
                sd->len_processed = 0;
                sd->pkt_processed = 0;
            }
        }
    }
}

 /***************
  * receiving
  ***************/

static uint32_t get_length(uint8_t len[4]) {
    uint32_t size;
    size = (uint32_t) len[0];
    size <<= 8;
    size += (uint32_t) len[1];
    size <<= 8;
    size += (uint32_t) len[2];
    size <<= 8;
    size += (uint32_t) len[3];
    return size;
}

static void get_timestamp (uint8_t timestamp[16], q_item_t * item){
    /* demarshal seconds and then nseconds both in BE format */
    int i;
    uint8_t byte;
    uint64_t byte8;
    byte8 = 0;
    for (i=0; i< 8; i++) {
        byte = timestamp[i];
        byte8 += byte;
        if (i<7) byte8 = byte8 << 8;
    }
    item->bignum_timestamp.sec = byte8;
    byte8 = 0;
    for (i=8; i< 16; i++) {
        byte = timestamp[i];
        byte8 += byte;
        if (i<15) byte8 = byte8 << 8;
    }
    item->bignum_timestamp.nsec = byte8;
    item->bignum_timestamp.positive = true;
}


static void init_receiving_function(void *d) {
    struct ciqs_t * ciqs = (struct ciqs_t *) d;
    struct receiving_data_t *rd = (struct receiving_data_t *) &(ciqs->data);
    rd->bytes_received = rd->bytes_to_receive = 0;
    rd->len_received = 0;
    rd->item = NULL;
}

 static void repeat_receiving_function(void *d) {
    bool q2_condition;
    struct ciqs_t * ciqs_ptr = (struct ciqs_t *) d;
    struct receiving_data_t *rd = (struct receiving_data_t *) &(ciqs_ptr->data.receive_data);
    if  (rd->lp->state >= LISA_CONNECTED) {
        if (rd->len_received < 4) {
            rd->len_received += lisa_recv(rd->lp, (char *) rd->len + rd->len_received, 4 - rd->len_received);
            q2log((Q2PRINT_PROCESSING ==2) && rd->len_received, "len_received: %d, lp:%p", rd->len_received, (void *) rd->lp);
        }
        if (rd->len_received == 4) {
            rd->bytes_to_receive = get_length(rd->len);
        }
        if ( (4 <= rd->len_received) && (rd->len_received < 20) )  {
            rd->len_received += lisa_recv(rd->lp, (char *) rd->timestamp + (rd->len_received - 4), 20 - rd->len_received);
            q2log((Q2PRINT_PROCESSING ==2) && rd->len_received, "len_received: %d, lp:%p", rd->len_received, (void *) rd->lp);
        }
        if (rd->len_received == 20) {
            rd->item = (q_item_t *) aq_get_tail(receive_inject_q);
            if (!(rd->item)) {
                printf("*** no space left in receive_inject_q, can't receive right now ...\n");
                return;
            }
            get_timestamp(rd->timestamp, rd->item);
        }
        if (rd->len_received >= 20) {
            if (!(rd->item)) return;
            q2log((Q2PRINT_PROCESSING == 2), "bytes to receive: %d, lp:%p", rd->bytes_to_receive, (void *) rd->lp);
            assert(rd->bytes_to_receive <= BUFFER_SIZE);
            rd->item->size = rd->bytes_to_receive;
            rd->bytes_received += lisa_recv(rd->lp, (char *) rd->item->buffer + rd->bytes_received, rd->bytes_to_receive - rd->bytes_received);
            assert(rd->bytes_received <= rd->bytes_to_receive);
            if (rd->bytes_received == rd->bytes_to_receive) {
                aq_put_tail(receive_inject_q);
                packets_received++;
                q2_set_condition(q2_condition, (Q2PRINT_PROCESSING && (Q2PRINT_BEACONS || !is_beacon(rd->item->buffer, rd->item->size))));
                q2log(q2_condition, "received %d (%d)", rd->bytes_received, packets_received);
                q2log_wftype(q2_condition, rd->item->buffer, rd->item->size, "received");
                rd->len_received = 0;
                rd->len_received = rd->bytes_received = rd->bytes_to_receive = 0;
                rd->item = NULL;
            }
        }
    }
}

/**************************************
    capture
 **************************************/

 static void init_capturing_function(void *d) {
    struct ciqs_t * ciqs = (struct ciqs_t *) d;
    pcap_t ** ptr2_pcap_handle = ciqs->data.pcap_data.pcap_handle_ptr;
    if (!*ptr2_pcap_handle) {
        q2log(Q2PRINT_CAPTURED, "opening pcap\n");
        *ptr2_pcap_handle = pcap_open_live(PCAP_DEVICE, BUFFER_SIZE, 1, 600, pcap_errbuf);
        if (!*ptr2_pcap_handle) {
            printf("error opening pcap for capture\n");
        }
        else pcap_setnonblock(*ptr2_pcap_handle, 1, pcap_errbuf);
    }
}

 static void repeat_capturing_function(void *d) {
    bool q2_condition;
    struct timespec now;
    struct ciqs_t * ciqs = (struct ciqs_t *) d;
    pcap_t ** ptr2_pcap_handle = (pcap_t **) ciqs->data.pcap_data.pcap_handle_ptr;
    q_item_t * new_item;
    struct pcap_pkthdr *pkt_header;
    const u_char *pkt_data;
    int rc;
    if (*ptr2_pcap_handle) {
        new_item = (q_item_t *) aq_get_tail(capture_send_q);
        if (!new_item) {
            printf("error, capture_send_q is full, can't process capture now\n");
        }
        else {
            rc = pcap_next_ex(*ptr2_pcap_handle, &pkt_header,  &pkt_data);
            if (rc>0) {
                if (clock_gettime(CLOCK_REALTIME, &now)<0) bignum_sec_assigns(&(new_item->bignum_timestamp), true, 0, 0);
                else bignum_sec_assigns(&(new_item->bignum_timestamp), true, now.tv_sec, now.tv_nsec);
                packets_captured++;
                #ifdef SKIP_ACKS
                if (is_ack((uint8_t *) pkt_data, pkt_header->caplen)) return;
                #endif
                new_item->size = (pkt_header->caplen < BUFFER_SIZE) ? pkt_header->caplen : BUFFER_SIZE;
                memcpy(new_item->buffer, pkt_data, new_item->size);
                aq_put_tail(capture_send_q);
                q2_set_condition(q2_condition, (Q2PRINT_PROCESSING && (Q2PRINT_BEACONS || is_beacon((uint8_t *) pkt_data, pkt_header->caplen))));
                q2log(q2_condition, "captured %d (%d)", pkt_header->caplen, packets_captured);
                q2log_wftype(q2_condition, (uint8_t *) pkt_data, pkt_header->caplen, "captured");
            }
        }
    }
    else printf("error: no pcap handle!\n");
 }


/**********************************************************
    injection
 **********************************************************/

static void init_injecting_function(void *d) {
    struct ciqs_t * ciqs = (struct ciqs_t *) d;
    pcap_t ** ptr2_pcap_handle = (pcap_t **) ciqs->data.pcap_data.pcap_handle_ptr;
    if (!*ptr2_pcap_handle) {
        *ptr2_pcap_handle = pcap_open_live(PCAP_DEVICE, BUFFER_SIZE, 1, 600, pcap_errbuf);
        if (!*ptr2_pcap_handle) {
            printf("error opening pcap for injection\n");
        }
    }
}

static void repeat_injecting_function(void *d) {
    bool q2_condition;
    struct ciqs_t * ciqs = (struct ciqs_t *) d;
    pcap_t ** ptr2_pcap_handle = (pcap_t **) ciqs->data.pcap_data.pcap_handle_ptr;
    if (!ptr2_pcap_handle) {
        printf("error: no open pcap handle for injection\n");
        return;
    }
    int rc;
    q_item_t *item;
    item = (q_item_t *) aq_get_head(receive_inject_q);
    if (item) {
        q2_set_condition(q2_condition, (Q2PRINT_PROCESSING && (Q2PRINT_BEACONS || !is_beacon(item->buffer, item->size))));
        q2log(q2_condition, "injecting packet %d", packets_injected);
        q2log_wftype(q2_condition, item->buffer, item->size, "injecting");
        rc = pcap_inject(*ptr2_pcap_handle, item->buffer, item->size);
        q2log(q2_condition, "injected packet %d", packets_injected);
        if (rc == PCAP_ERROR) printf("error injecting: %s\n", pcap_geterr(*ptr2_pcap_handle));
        else {
            packets_injected++;
            q2log(q2_condition, "injected %d (%d)", rc, packets_injected);
            q2log_wftype(Q2PRINT_INJECTED, item->buffer, item->size, "injected");
            profiling_update_lag_time(&(item->bignum_timestamp));
        }
        aq_used_head(receive_inject_q);
    }
 }

/**************************************
    capture via netlink
 **************************************/

 static void init_nlcapturing_function(void *d) {
    printf("nl started\n");
}

 static void repeat_nlcapturing_function(void *d) {
    cinl_run();
 }

 /**************************************
    inject via netlink (TODO)
 **************************************/

 static void init_nlinjecting_function(void *d) {
    return;
}

 static void repeat_nlinjecting_function(void *d) {
    return;
 }



/*
 * example of uint64_t printing:
 *   printf("injected from: %#018" PRIx64 ".%#018" PRIx64 " \n", item->bignum_timestamp.sec, item->bignum_timestamp.nsec);
 *   printf("injected from: %010ld.%06ld \n", item->bignum_timestamp.sec, item->bignum_timestamp.nsec);
 */

