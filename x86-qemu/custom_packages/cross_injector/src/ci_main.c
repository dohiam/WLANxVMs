/*!
 * @file src/ci_main.c
 * @brief the main function for the application
 * @details
 * Accepts several options but the only "production" option is c. The other options are for unit testing different parts of the application.
 * Passing the single character c starts all the queues and workers for a client.
 */

#define QUEUES_TEST

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <pcap.h>
#include "ci_main.h"
#include "ci_queues.h"
#include "ci_client_server.h"
#include "ci_workers.h"
#include "ci_nl.h"
#include "utilities.h"
#include "item.h"

static enum mode_t {none, client_server, nl_test} mode = none;

static lisa l_server;
static lisa *lp_server=&l_server;

static lisa l_client;
static lisa *lp_client=&l_client;

static pcap_t *pcap_capture_handle = NULL;
//static pcap_t *pcap_injection_handle;

void print_error(int x) {
    switch(x) {
        case NO_ERROR: q2print("OK - no error\n"); break;
        case OK_DUPLICATE: q2print("Duplicate packet\n"); break;
        case ERROR_PKT_NOT_VALID: q2print("ERROR: packet not valid\n"); break;
        case ERROR_LIBPCAP: q2print("ERROR: error returning from libpcap\n"); break;
        default: q2print("UNKNOWN ERROR!!!\n");
    }
}

/*
 * catch control-c
 */
void sig_handler(int signo)
{
  if (signo == SIGINT) {
        switch (mode) {
        case client_server:
            printf("packets_nlcaptured: %d\n", cinl_received());
            ciqs_print_stats();
            printf("stopping queues ...\n");
            ciqs_stop_all();
            printf("stopping workers ...\n");
            stop_printing_worker();
            #ifdef USE_FS_MALLOC
            fs_malloc_close();
            #endif
            break;
        case nl_test:
            printf("packets_nlcaptured: %d\n", cinl_received());
            ciqs_print_stats();
            break;
        case none:
            break;
        }
        exit(0);
  }
}

static void start_client(int use_nl) {
    q2print("starting queues\n");
    #ifdef USE_FS_MALLOC
    if (fs_malloc_open()<0) {
      printf("failed to allocate memory pool\n");
      return;
    }
    else printf("using fs_malloc\n");
    #endif
    ciqs_init_queues();
    ciqs_start(sendq, lp_client);
    ciqs_start(receiveq, lp_client);
    if (use_nl) ciqs_start(nlcaptureq, NULL);
    else ciqs_start(captureq, &pcap_capture_handle);
    ciqs_start(injectionq, &pcap_capture_handle);
    printf("queues ready\n");
    printf("starting client\n");
    #ifdef USE_VSOCK
    ci_start_client(TEST_SERVER_ADDR, SERVER_PORT, lp_client, 1);
    #else
    ci_start_client(TEST_SERVER_ADDR, SERVER_PORT, lp_client, 0);
    #endif // USE_VSOCK
}

static void start_server(int use_nl) {
    q2print("starting queues\n");
    ciqs_init_queues();
    ciqs_start(altsendq, lp_server);
    ciqs_start(altreceiveq, lp_server);
    if (use_nl) ciqs_start(nlcaptureq, NULL);
    else ciqs_start(captureq, &pcap_capture_handle);
    ciqs_start(injectionq, &pcap_capture_handle);
    q2print("queues ready\n");
    q2print("starting workers\n");
    q2print("workers started\n");
    q2print("starting server\n");
    #ifdef USE_VSOCK
    ci_start_server(SERVER_PORT, lp_server, 1);
    #else
    ci_start_server(SERVER_PORT, lp_server, 0);
    #endif // USE_VSOCK
}


/*
 * main
 */

int main(int argc, char **argv) {

    if (argc < 2) {
      printf("invoke with either 'c' or 's' as a command line parameter (client or server)\n");
      return -1;
    }
    start_printing_worker();
	switch (argv[1][0]) {

	    case 'n': mode = nl_test;
                  ciqs_init_queues();
                  ciqs_start(nlcaptureq, NULL);
	              signal (SIGINT,sig_handler);
                  while (1) { sleep(1); }
                  break;

	    case 'o': mode = client_server;
                  start_client(1);
	              signal (SIGINT,sig_handler);
                  while (1) { sleep(1); }
                  break;

	    case 'c': mode = client_server;
                  start_client(0);
	              signal (SIGINT,sig_handler);
                  while (1) { sleep(1); }
                  break;

	    case 's': mode = client_server;
                  start_server(0);
	              signal (SIGINT,sig_handler);
                  while (1) { sleep(1); }
                  break;

	    default: printf("invoke with either 'c' or 's' as a command line parameter (client or server)\n");

	}
}

