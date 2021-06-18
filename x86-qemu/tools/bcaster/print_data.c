#include "print_data.h"
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <stdarg.h>
#include "bit_things.h"
#include "queue.h"
#include "pkt.h"
#include <inttypes.h>

#define DATA_SIZE 100
#define NUM_DATA 100
#define SLEEP_TIME 10000

#define Black  "\033[0;30m"
#define Red    "\033[0;31m"
#define Green  "\033[0;32m"
#define Yellow "\033[0;33m"
#define Blue   "\033[0;34m"
#define Purple "\033[0;35m"
#define Cyan   "\033[0;36m"
#define White  "\033[0;37m"

void change_color(int color) {
    switch (color) {
        case RED:    printf(Red);    break;
        case GREEN:  printf(Green);  break;
        case YELLOW: printf(Yellow); break;
        case BLUE:   printf(Blue);   break;
        case PURPLE: printf(Purple); break;
        case CYAN:   printf(Cyan);   break;
        default:     printf(White);  break;
    };
}

typedef struct print_data_s {
    int client;
    int len;
    int color;
    struct timeval timestamp;
    uint8_t data[DATA_SIZE];
} print_data_t;

aq_type * print_q;

pthread_t print_data_thread;

static bool ignore_beacons;
static bool print_data_bytes;

void print_data_add_pkt(int client, uint8_t *pkt, uint32_t len, int color) {
    int size = len < DATA_SIZE ? len : DATA_SIZE;
    print_data_t * pd = (print_data_t *) aq_get_tail(print_q);
    if (pd != NULL) {
        pd->client = client;
        pd->len = len;
        pd->color = color;
        gettimeofday(&(pd->timestamp), NULL);
        memcpy(pd->data, pkt, size);
        aq_put_tail(print_q);
    }
}

void * print_data_worker (void * d) {
    frame_control_t fc;
    char str[25];
    uint8_t * data_start;
    uint8_t * pkt_start;
    int data_len;
    int pkt_len;
    print_data_t * pd;
   while (true) {
        while ((pd = (print_data_t *) aq_get_head(print_q)) != NULL) {
            change_color(pd->color);
            if (pd->len < 4) {
                printf("error: pkt too short to even get length from!\n");
            }
            else {
                pkt_len = pkt_get_length(pd->data);
                pkt_start = pd->data+20;
                if (pkt_len != (pd->len - 20)) {
                    printf("\n partial or multiple packet received: expected %d but got %d\n", pkt_len, pd->len);
                }
                if (!ignore_beacons || !pkt_is_beacon(pkt_start, pkt_len)) {
                    printf("%10ld.%-6ld  ", pd->timestamp.tv_sec, pd->timestamp.tv_usec);
                    printf("%d ", pd->client);
                    pkt_get_type_string(pkt_start, pkt_len, str, 25);
                    printf("%s ", str);
                    if (pkt_is_data(pkt_start, pkt_len, &data_start, &data_len)) {
                        pkt_get_llc_type_string(data_start, data_len, str, 25);
                        printf("%-5s ", str);
                        pkt_get_ether_type_string(data_start, data_len, str, 25);
                        printf("%-7s ", str);
                        // skip over LLC frame
                        if (print_data_bytes) {
                            for (int i=0; i<(data_len-6); i++) {
                                printf("%02X:", data_start[i+6]);
                            }
                        }
                    }
                    printf("\n");
                    pkt_get_frame_control(&fc, pkt_start, pkt_len);
                    if (fc.PS) printf("POWER SAVE SET\n");
                }
            }
            aq_used_head(print_q);
        }
        usleep(SLEEP_TIME);
    }
}

void print_data_init(bool ignore_beacons_arg, bool print_data_bytes_arg) {
    ignore_beacons = ignore_beacons_arg;
    print_data_bytes = print_data_bytes_arg;
    if (aq_new(&print_q, sizeof(print_data_t), NUM_DATA))
        pthread_create(&(print_data_thread), NULL, print_data_worker, NULL);
    else
        printf("error initializing print data\n");
}


