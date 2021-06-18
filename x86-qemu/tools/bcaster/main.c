#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include "lisa.h"
#include "looper.h"
#include "print_data.h"
#include "pkt.h"

#define PORT 9991
#define MAX_SIZE 3000

#define STOP_MSG "***STOP***NOW***"
#define STOP_MSG_LEN sizeof(STOP_MSG)

int packets_received = 0;
int packets_sent = 0;

static lisa l_server;
static lisa *lp=&l_server;

/*******************************************************
 * receive buffers (ADO)
 ******************************************************/

uint8_t temp_buffer[MAX_SIZE];

typedef struct client_data_s {
    int client_fd;
    int color;
    uint32_t bytes_expected;
    uint32_t bytes_received;  /* also think of this as "bytes remaining to be processed" */
    uint8_t  buffer[MAX_SIZE];
    uint8_t *next_pkt;
 } client_data_t;

 struct buffers_s {
    pthread_mutex_t lock;
    int num_clients;
    client_data_t client[MAX_CLIENTS];
 } buffers;

 void buffers_init() {
    pthread_mutex_init(&(buffers.lock), NULL);
    buffers.num_clients = 0;
 }

 int client_for_fd(int client_fd) {
    for (int i=0; i < buffers.num_clients; i++) {
        if (buffers.client[i].client_fd == client_fd) return i;
    }
    return -1;
 }

/*******************************************************
 * utilities
 ******************************************************/
#include <stdio.h>
#include <signal.h>

/*
 * catch control-c
 */
void sig_handler(int signo)
{
  if (signo == SIGINT) {
        change_color(DEFAULT_COLOR);
        printf("\n");
        printf("packets received: %d\n", packets_received);
        printf("packets sent: %d\n",     packets_sent);
        pkt_close_file();
        exit(0);
  }
}


/*******************************************************
 * listener
 ******************************************************/
struct looper_t listener;
static int next_color;

static void init_listener(void *d) {
    lisa_open(lp);
    lisa_set_to_vsock(lp);
    lisa_set_wait_time(lp, 2);
    buffers_init();
    next_color = FIRST_COLOR;
    printf("listener initialized\n");
}

#define NEXT_CLIENT buffers.client[buffers.num_clients]

static void repeat_listening_function(void *d) {
    int num_clients;
    num_clients = lp->num_clients;
    lisa_accept(lp, PORT);
    if (lp->num_clients > num_clients) {
        change_color(next_color);
        printf("Received connection, fd=%d as client number %d\n", lp->client_fd, num_clients);
        pthread_mutex_lock(&(buffers.lock));
        NEXT_CLIENT.client_fd = lp->client_fd;
        NEXT_CLIENT.bytes_expected = 0;
        NEXT_CLIENT.bytes_received = 0;
        NEXT_CLIENT.next_pkt = NEXT_CLIENT.buffer;
        NEXT_CLIENT.color = next_color;
        if (next_color == LAST_COLOR) next_color = FIRST_COLOR;
        else next_color++;
        buffers.num_clients++;
        pthread_mutex_unlock(&(buffers.lock));
    }
}

static void final_listener(void *d) {
    for (int i=0; i<lp->num_clients; i++) {
        lisa_close(lp);
    }
}


/*******************************************************
 * receiver
 ******************************************************/
struct looper_t receiver;

static int match_end(char * s) {
    if (strnlen(s, STOP_MSG_LEN) != STOP_MSG_LEN) return 0;
    return !strncmp(s, STOP_MSG, STOP_MSG_LEN);
}

static void process_pkt(uint8_t * pkt, uint32_t pkt_len, int clnum) {
    packets_received++;
    if (match_end( (char *) pkt)) {
        lisa_close_client(lp);
    }
    else {
        lisa_cast(lp, (char *) pkt, pkt_len);
        packets_sent++;
        print_data_add_pkt(clnum, pkt, pkt_len, buffers.client[clnum].color);
        pkt_write_file(pkt+20, pkt_len-20);  /* don't include the len bytes or timestamp in packet capture */
    }
}

#define CLIENT buffers.client[clnum]
/* receive into the client buffer from the select (part 1),
 * if received length and received entire packet then process
 */
static void repeat_receiving_function(void *d) {
    int num_clients;
    pthread_mutex_lock(&(buffers.lock));
    num_clients = buffers.num_clients;
    pthread_mutex_unlock(&(buffers.lock));
    if (num_clients == 0) return;
    int clfd = lisa_recv_part1(lp);
    if (clfd < 0) return;
    pthread_mutex_lock(&(buffers.lock));
    int clnum = client_for_fd(clfd);
    uint8_t * buffer_start = CLIENT.buffer + CLIENT.bytes_received;
    uint32_t buffer_remaining = MAX_SIZE - CLIENT.bytes_received;
    CLIENT.bytes_received += lisa_recv_part2(lp, (char *) buffer_start, buffer_remaining);
    while ((CLIENT.bytes_received >= 4) && (CLIENT.bytes_expected <= CLIENT.bytes_received)) {
        if ( (CLIENT.bytes_expected == 0) && (CLIENT.bytes_received >= 4) ) {
            CLIENT.bytes_expected = pkt_get_length(CLIENT.buffer) +20;  /* include len bytes and timestamp */
        }
        if (CLIENT.bytes_expected && (CLIENT.bytes_expected <= CLIENT.bytes_received )) {
            process_pkt(CLIENT.next_pkt, CLIENT.bytes_expected, clnum);
            CLIENT.bytes_received -= CLIENT.bytes_expected;
            CLIENT.next_pkt += CLIENT.bytes_expected;
            CLIENT.bytes_expected = 0;
        }
    }
    if (CLIENT.bytes_received > 0) { /* relocate remaining bytes from next_pkt to front; assume this is rarely needed */
        for (int i=0; i<CLIENT.bytes_received; i++) CLIENT.buffer[i] = CLIENT.next_pkt[i];
    }
    CLIENT.next_pkt = CLIENT.buffer;
    pthread_mutex_unlock(&(buffers.lock));
}

/*******************************************************
 * main
 ******************************************************/
int main() {
    printf("initializing\n");
    looper_init(&listener);
    looper_init(&receiver);
    listener.function_to_run_first = init_listener;
    listener.function_to_repeat = repeat_listening_function;
    listener.function_to_run_last = final_listener;
    receiver.function_to_run_first = NULL;
    receiver.function_to_repeat = repeat_receiving_function;
    receiver.function_to_run_last = NULL;
    if (!pkt_open_file()) printf("error opening pcap dump file\n");
    else printf("pcap dump file opened\n");
    printf("starting threads\n");
    print_data_init(true, false);
    looper_start(&listener);
    looper_start(&receiver);
    signal (SIGINT,sig_handler);
    while (1) { sleep(1); }
}
