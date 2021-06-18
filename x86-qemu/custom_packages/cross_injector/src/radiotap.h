/*!
 * @file src/radiotap.h
 * @brief defines some utility functions around radiotap headers.
 * @details
 * The motivation for these utilities were to be able to add or modify basic radiotap headers as needed since this header is required
 * to inject packets. In practice, this turns out not to be needed as simply copying the radiortap header that is captured along with
 * the rest of the packet seems to work fine.
 *
 * fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).
 */

#ifndef RADIOTAP_H
#define RADIOTAP_H

#include "item.h"
#include <stdint.h>

#define RT_SIMPLE_HEADER { 0x00,0x00,0x0b,0x00,0x04,0x0C,0x00,0x00,0x6C,0x0C,0x01 }

struct rt_data_t {
    uint8_t  flags;
    uint8_t  rate;
    uint8_t  antenna;
    uint16_t frequency;
    uint16_t channel_flags;
    uint64_t tsft_time;
};

#define RT_HEADER_SIZE 23
typedef uint8_t rt_header[RT_HEADER_SIZE];

void rt_print_hdr(rt_header hdr);

void rt_make_header_from_data(struct rt_data_t *data, rt_header hdr);

void rt_set_defaults(struct rt_data_t *data);

//skips over radiotap header
void rt_get_payload(struct item_t * item, uint8_t ** payload, uint32_t * payload_size);

//changes rt header in item to the default one
void rt_to_default(struct item_t * item);

void rt_set_channel_header(uint8_t *hdr, unsigned int freq);
uint8_t * rt_add_channel_header(uint8_t *data, unsigned int data_len, unsigned int freq);
#define RT_CHANNEL_HEADER_SIZE 12


#endif // RADIOTAP_H
