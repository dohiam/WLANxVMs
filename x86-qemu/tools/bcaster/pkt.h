#ifndef PKT_H
#define PKT_H

#include <stdint.h>
#include <stdbool.h>

void pkt_get_type_string(uint8_t *pkt, uint32_t size, char * type_string, int len);
void pkt_get_llc_type_string(uint8_t *data_start, uint32_t data_size, char * type_string, int len);
void pkt_get_ether_type_string(uint8_t *data_start, uint32_t data_size, char * type_string, int len);

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

uint32_t pkt_get_length(uint8_t len[4]);

void pkt_get_frame_control(frame_control_t *fc, uint8_t *pkt, uint32_t size);

bool pkt_is_ack(uint8_t *pkt, int size);

bool pkt_is_beacon(uint8_t *pkt, int size);

bool pkt_is_data(uint8_t *pkt, int size, uint8_t ** data_ptr_h, int * data_len_h);

void pkt_get_llc_type_string(uint8_t *data_start, uint32_t data_size, char * type_string, int len);

int  pkt_open_file();
void pkt_write_file(uint8_t *pkt, uint32_t pkt_len);
void pkt_close_file();

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

#define SNAP_TYPE_IPV4              0x0800
#define SNAP_TYPE_IPV6              0x86DD
#define SNAP_TYPE_ARP               0x0806
#define SNAP_TYPE_EAP               0x888E

#define ETHER_TYPE_ICMPV4       0x01
#define ETHER_TYPE_IGMPV4       0x02
#define ETHER_TYPE_TCP          0x06
#define ETHER_TYPE_UDP          0x11
#define ETHER_TYPE_IPV6         0x29
#define ETHER_TYPE_ROUTING      0x2B
#define ETHER_TYPE_RSVP         0x2E
#define ETHER_TYPE_ICMPV6       0x3A
#define ETHER_TYPE_NONE         0x3B

#endif
