/*!
 * @file src/utilities.h
 * @brief Defines severy utility functions around debug printing and an alternative memory management using a fixed size memory pool.
 * @details
 * The debug printing can either block the calling thread (using standard printf) or it can queue the formatted print along with the
 * current time tag and print it on a separate thread in order to minimize the impact on the calling thread. Note that the
 * q2print_wftype function provides some useful decoding of the Wi-Fi packet header which can make it easy to see the back and forth
 * between the AP and the STA (such as the authentication and association stages.
 *
 * fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).
 */

#ifndef UTILITIES_H
#define UTILITIES_H

#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include "debug.h"

#ifdef DEBUG
#define q2_set_condition(var, value) var = value
#else
#define q2_set_condition(var, value) var = true
#endif

void start_printing_worker();

void stop_printing_worker();

void q2print(const char *fmt, ...);

void q2log(bool condition, const char *fmt, ...);

void q2print_hex(uint8_t *data, int data_len);

void q2log_hex(bool condition, uint8_t *data, int data_len);

void q2print_wftype(uint8_t *wfpkt, uint32_t size, char * msg);

void q2log_wftype(bool condition, uint8_t *wfpkt, uint32_t size, char * msg);

void force_powersave_flag_off(uint8_t *wfpkt, uint32_t size);

/* fixed size memory allocation based on assumption is that free is shortly after allocation (i.e., for very temporary memory usage) */
#define FS_MAX_SIZE 1500
#define FS_MAX_NUM  1000
int  fs_malloc_open(); /* negative return value is failure */
void fs_malloc_close();
void * fs_malloc(unsigned int size);
void fs_free(void * ptr);

uint32_t fcs(uint8_t *wfpkt, uint32_t size);

struct frame_control_s {
    uint8_t version;
    uint8_t type_subtype;
    bool toDS;
    bool FromDS;
    bool morefrag;
    bool retry;
    bool PS;
    bool moredata;
    bool protected_MF;
    bool order;
    uint8_t dest[6];
};

typedef struct frame_control_s frame_control_t;

//Note: wfpkt should contain the radiotap header
void get_frame_control(frame_control_t *fc, uint8_t *wfpkt, uint32_t size);

bool is_ack(uint8_t *data, int size);
bool is_beacon(uint8_t *data, int size);

#define ASSOCIATION_REQUEST     0x00
#define ASSOCIATION_RESPONSE    0x01
#define REASSOCIATION_REQUEST   0x02
#define REASSOCIATION_RESPONSE  0x03
#define PROBE_REQUEST           0x04
#define PROBE_RESPONSE          0x05
#define TIMING_ADVERTISEMENT    0x06
#define RESERVED0               0x07
#define BEACON                  0x08
#define ATIM                    0x09
#define DISASSOCIATION          0x0A
#define AUTHENTICATION          0x0B
#define DEAUTHENTICATION        0x0C
#define ACTION                  0x0D
#define RESERVED1               0x0E ... 0x17
#define BLOCK_ACK_REQUEST       0x18
#define BLOCK_ACK               0x19
#define PS_POLL                 0x1A
#define RTS                     0x1B
#define CTS                     0x1C
#define ACK                     0x1D
#define CF_END                  0x1E
#define CF_END_CF_ACK           0x1F
#define DATA                    0x20
#define DATA_CF_ACK             0x21
#define DATA_CF_POLL            0x22
#define DATA_CF_ACK_CF_POLL     0x23
#define NULL_FRAME              0x24
#define CF_ACK                  0x25
#define CF_POLL                 0x26
#define CF_ACK_CF_POLL          0x27
#define QOS_DATA                0x28
#define QOS_DATA_CF_ACK         0x29
#define QOS_DATA_CF_POLL        0x2A
#define QOS_DATA_CF_ACK_CF_POLL 0x2B
#define QOS_NULL                0x2C
#define RESERVED2               0x2D
#define QOS_CF_POLL_NO_DATA     0x2E
#define QOS_CF_ACK_NO_DATA      0x2F
#define RESERVED3               0x30 ... 0x3F
#define UNKNOWN_TYPE_SUBTYPE    0x40 ... 0xFF

#endif // UTILITIES_H
