/*!
 * @file src/radiotap.c
 * @brief Implements some utility functions around radiotap headers.
 *
 * fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).
 */
#include "radiotap.h"
#include "history.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#define DEFAULT_CHANNEL_FLAGS 0x00c0

void rt_print_hdr(rt_header hdr) {
    printf("RT HDR: ");
    for (int i=0; i<RT_HEADER_SIZE; i++) printf("%X ", hdr[i]);
    printf("\n");
}


uint8_t byte_of_u64(int byte_position, uint64_t u64) {
    for (int i=0; i< byte_position; i++) u64 = u64 / 256;
    return u64 & 0x00000000000000FF;
}


void rt_make_header_from_data(struct rt_data_t *data, rt_header hdr) {
    hdr[0] = 0;  // version
    hdr[1] = 0;  // pad
    hdr[2] = 23; // len low byte
    hdr[3] = 0;  // len high byte
    /* flags: bit 0: tsft, bit 1: flags, bit 2: rate, bit 3: channel, bit 11: antenna */
    hdr[4] = 0x0F;
    hdr[5] = 0x08;
    hdr[6] = 0;
    hdr[7] = 0;
    /* tsft */
    hdr[8] =  byte_of_u64(0,data->tsft_time);
    hdr[9] =  byte_of_u64(1,data->tsft_time);
    hdr[10] = byte_of_u64(2,data->tsft_time);
    hdr[11] = byte_of_u64(3,data->tsft_time);
    hdr[12] = byte_of_u64(4,data->tsft_time);
    hdr[13] = byte_of_u64(5,data->tsft_time);
    hdr[14] = byte_of_u64(6,data->tsft_time);
    hdr[15] = byte_of_u64(7,data->tsft_time);
    /* flags */
    hdr[16] = data->flags;
    /* rate */
    hdr[17] = data->rate;
    /* channel */
    hdr[18] = data->frequency & 0x00FF;
    hdr[19] = (data->frequency / 256) & 0x00FF;
    hdr[20] = data->channel_flags & 0x00FF;
    hdr[21] = (data->channel_flags /256) & 0x00FF;
    /* antenna */
    hdr[22] = data->antenna;
}

uint8_t * rt_add_channel_header(uint8_t *data, unsigned int data_len, unsigned int freq) {
    uint8_t * new_data;
    uint8_t hdr[RT_CHANNEL_HEADER_SIZE];

    hdr[0] = 0;  // version
    hdr[1] = 0;  // pad

    hdr[2] = RT_CHANNEL_HEADER_SIZE; // len low byte
    hdr[3] = 0;  // len high byte

    /* flags: bit 3: channel */
    hdr[4] = 0x08;
    hdr[5] = 0;
    hdr[6] = 0;
    hdr[7] = 0;

    /* channel */
    hdr[8] = freq & 0x00FF;
    hdr[9] = (freq / 256) & 0x00FF;
    hdr[10] = DEFAULT_CHANNEL_FLAGS & 0x00FF;
    hdr[11] = (DEFAULT_CHANNEL_FLAGS /256) & 0x00FF;

    new_data = malloc(data_len + RT_CHANNEL_HEADER_SIZE);
    if (new_data) {
        memcpy(new_data, hdr, RT_CHANNEL_HEADER_SIZE);
        memcpy(new_data+RT_CHANNEL_HEADER_SIZE, data, data_len);
    }
    return new_data;
}

void rt_set_channel_header(uint8_t *hdr, unsigned int freq) {

    hdr[0] = 0;  // version
    hdr[1] = 0;  // pad

    hdr[2] = RT_CHANNEL_HEADER_SIZE; // len low byte
    hdr[3] = 0;  // len high byte

    /* flags: bit 3: channel */
    hdr[4] = 0x08;
    hdr[5] = 0;
    hdr[6] = 0;
    hdr[7] = 0;

    /* channel */
    hdr[8] = freq & 0x00FF;
    hdr[9] = (freq / 256) & 0x00FF;
    hdr[10] = DEFAULT_CHANNEL_FLAGS & 0x00FF;
    hdr[11] = (DEFAULT_CHANNEL_FLAGS /256) & 0x00FF;

}

void rt_set_defaults(struct rt_data_t *data) {
    data->flags = 0;
    data->rate = 108;
    data->antenna = 1;
    data->frequency = 2437;
    data->channel_flags = 0x00c0;
    data->tsft_time = 0;
}

void rt_to_default(struct item_t * item) {
    int i;
    uint32_t payload_size;
    uint8_t * payload;
    uint8_t * new_buffer;
    struct rt_data_t default_hdr_data;
    rt_header default_hdr;
    rt_set_defaults(&default_hdr_data);
    rt_make_header_from_data(&default_hdr_data, default_hdr);
    rt_get_payload(item, &payload, &payload_size);
    new_buffer = malloc(payload_size + RT_HEADER_SIZE);
    if (new_buffer) {
        for (i=0; i<RT_HEADER_SIZE; i++) new_buffer[i] = default_hdr[i];
        for (; i<payload_size+RT_HEADER_SIZE; i++) new_buffer[i] = payload[i-RT_HEADER_SIZE];
    }
    free(item->buffer);
    item->buffer = new_buffer;
}

 void rt_get_payload(struct item_t * item, uint8_t ** payload, uint32_t * payload_size) {
     *payload_size = item->size - item->buffer[2];
     *payload = item->buffer + item->buffer[2];
 }

/******************************************************************************************************/

/*

static  uint8_t default_radiotap_header[] = { 0x00,0x00,0x0b,0x00,0x04,0x0C,0x00,0x00,0x6C,0x0C,0x01 };
#define DEFAULT_HEADER_SIZE 11

static void rt_chg2default_radiotap_header (struct item_t * item) {
    uint8_t * new_pkt;
    uint32_t new_hdr_size;
    uint32_t old_hdr_size;
    uint32_t old_data_size;
    uint32_t new_pkt_size;
    new_hdr_size = DEFAULT_HEADER_SIZE;
    old_hdr_size = item->buffer[2];
    assert(old_hdr_size < item->size);
    old_data_size = item->size - old_hdr_size;
    new_pkt_size = old_data_size + new_hdr_size;
    new_pkt = malloc(new_pkt_size);
    if (!new_pkt) {
        printf("error allocating memory for changing header\n");
        return;
    }
    memcpy(new_pkt, default_radiotap_header, new_hdr_size);
    memcpy(new_pkt+new_hdr_size, (item->buffer)+old_hdr_size, old_data_size);
    free(item->buffer);
    item->buffer = new_pkt;
    item->size = new_pkt_size;
}

*/
