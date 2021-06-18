
#include "pkt.h"
#include "bit_things.h"
#include <stdio.h>
#include <sys/time.h>
#include <pcap.h>

uint32_t pkt_get_length(uint8_t len[4]) {
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

// Note about bit order of things in the frame control field:
// field order: version, type, subtype followed by toDS, FromDS, morefrag, retry, PS, moredata, protected, order
// so MSB byte order is subtype, type, version, order, protected, moredata, PS, retry, morefrag, FromDS, toDS
// fc[7..0] == subtype [7..4] + type[3..2] + version[1..0]
// flags[7..0] == order[7] + protected [6] + more_data[5] + PS[4] + retry[3] + more_frag[2] + FromDS, toDS

void pkt_get_frame_control(frame_control_t *fc, uint8_t *wfpkt, uint32_t size) {
    unsigned int radiotap_len = wfpkt[2] + (wfpkt[3] << 8);
    uint8_t fc_field = wfpkt[radiotap_len];
    uint8_t flags = wfpkt[radiotap_len+1];
    fc->version = getasb(fc_field,1,1) + getasb(fc_field,0,0);
    fc->type_subtype = getasb(fc_field,3,5) + getasb(fc_field,2,4) +
                       getasb(fc_field,7,3) + getasb(fc_field,6,2) + getasb(fc_field,5,1) + getasb(fc_field,4,0);
    //if (fc->type_subtype != 0x08) q2print("fc_field: %02x, flags: %02x, TS: %02x", fc_field, flags, fc->type_subtype);
    fc->toDS         = getb(flags,0);
    fc->FromDS       = getb(flags,1);
    fc->morefrag     = getb(flags,2);
    fc->retry        = getb(flags,3);
    fc->PS           = getb(flags,4);
    fc->moredata     = getb(flags,5);
    fc->protected_MF = getb(flags,6);
    fc->order        = getb(flags,7);
    for (int i=0; i<6; i++) fc->dest[i] = wfpkt[radiotap_len+4+i];
}


bool pkt_is_ack(uint8_t *pkt, int size) {
  frame_control_t fc;
  pkt_get_frame_control(&fc, pkt, size);
  return (fc.type_subtype == ACK);
}

bool pkt_is_beacon(uint8_t *pkt, int size) {
  frame_control_t fc;
  pkt_get_frame_control(&fc, pkt, size);
  return (fc.type_subtype == BEACON);
}

bool pkt_is_data(uint8_t *pkt, int size, uint8_t ** data_ptr_h, int * data_len_h) {
    frame_control_t fc;
    unsigned int radiotap_len;
    pkt_get_frame_control(&fc, pkt, size);
    bool ret;
    if (fc.type_subtype == DATA ||
        fc.type_subtype == DATA_CF_ACK ||
        fc.type_subtype == DATA_CF_POLL ||
        fc.type_subtype == DATA_CF_ACK_CF_POLL ||
        fc.type_subtype == QOS_DATA ||
        fc.type_subtype == QOS_DATA_CF_ACK ||
        fc.type_subtype == QOS_DATA_CF_POLL ||
        fc.type_subtype == QOS_DATA_CF_ACK_CF_POLL) ret = true;
    else ret = false;
    if (data_ptr_h != NULL && data_len_h != NULL) {
        radiotap_len = pkt[2] + (pkt[3] << 8);
        *data_ptr_h = pkt+radiotap_len+24;
        *data_len_h = size-radiotap_len-24;
    }
    return ret;
}

void pkt_get_type_string(uint8_t *pkt, uint32_t size, char * type_string, int len) {
    frame_control_t fc;
    pkt_get_frame_control(&fc, pkt, size);
    switch(fc.type_subtype) {
        case ASSOCIATION_REQUEST:     snprintf(type_string, len, "association request"); break;
        case ASSOCIATION_RESPONSE:    snprintf(type_string, len, "association response"); break;
        case REASSOCIATION_REQUEST:   snprintf(type_string, len, "reassociation request"); break;
        case REASSOCIATION_RESPONSE:  snprintf(type_string, len, "reassociation response"); break;
        case PROBE_REQUEST:           snprintf(type_string, len, "probe request"); break;
        case PROBE_RESPONSE:          snprintf(type_string, len, "probe response"); break;
        case TIMING_ADVERTISEMENT:    snprintf(type_string, len, "timing advertisement"); break;
        case BEACON:                  snprintf(type_string, len, "beacon"); break;
        case ATIM:                    snprintf(type_string, len, "ATIM"); break;
        case DISASSOCIATION:          snprintf(type_string, len, "disassocation"); break;
        case AUTHENTICATION:          snprintf(type_string, len, "authentication"); break;
        case DEAUTHENTICATION:        snprintf(type_string, len, "deauthentication"); break;
        case ACTION:                  snprintf(type_string, len, "action"); break;
        case RESERVED0:
        case RESERVED1:
        case RESERVED2:
        case RESERVED3:               snprintf(type_string, len, "reserved"); break;
        case BLOCK_ACK_REQUEST:       snprintf(type_string, len, "block ack request"); break;
        case BLOCK_ACK:               snprintf(type_string, len, "block ack"); break;
        case PS_POLL:                 snprintf(type_string, len, "PS poll"); break;
        case RTS:                     snprintf(type_string, len, "RTS"); break;
        case CTS:                     snprintf(type_string, len, "CTS"); break;
        case ACK:                     snprintf(type_string, len, "ACK"); break;
        case CF_END:                  snprintf(type_string, len, "CF-end"); break;
        case CF_END_CF_ACK:           snprintf(type_string, len, "CF-end+CF-ack"); break;
        case DATA:                    snprintf(type_string, len, "Data"); break;
        case DATA_CF_ACK:             snprintf(type_string, len, "Data+CF-ack"); break;
        case DATA_CF_POLL:            snprintf(type_string, len, "Data+CF-poll"); break;
        case DATA_CF_ACK_CF_POLL:     snprintf(type_string, len, "Data+CF-ack+CF-poll"); break;
        case NULL_FRAME:              snprintf(type_string, len, "null"); break;
        case CF_ACK:                  snprintf(type_string, len, "CF-ack"); break;
        case CF_POLL:                 snprintf(type_string, len, "CF-poll"); break;
        case CF_ACK_CF_POLL:          snprintf(type_string, len, "CF-ack+CF-poll"); break;
        case QOS_DATA:                snprintf(type_string, len, "QOS data"); break;
        case QOS_DATA_CF_ACK:         snprintf(type_string, len, "QOS data + CF-ack"); break;
        case QOS_DATA_CF_POLL:        snprintf(type_string, len, "QOS data + CF-poll"); break;
        case QOS_DATA_CF_ACK_CF_POLL: snprintf(type_string, len, "QOS data + CF-ack + CF-poll"); break;
        case QOS_NULL:                snprintf(type_string, len, "QOS null"); break;
        case QOS_CF_POLL_NO_DATA:     snprintf(type_string, len, "QOS +CF-poll (no data)"); break;
        case QOS_CF_ACK_NO_DATA:      snprintf(type_string, len, "QOS +CF-ack (no data)"); break;
        case UNKNOWN_TYPE_SUBTYPE:    snprintf(type_string, len, "? type subtype %02X ??", fc.type_subtype); break;
    }
 }

void pkt_get_llc_type_string(uint8_t *data_start, uint32_t data_size, char * type_string, int len) {
    unsigned int type_start = 6;
    uint16_t     snap_type = (data_start[type_start] << 8) + data_start[type_start+1];
    switch (snap_type) {
        case SNAP_TYPE_IPV4:   snprintf(type_string, len, "IPv4");  break;
        case SNAP_TYPE_IPV6:   snprintf(type_string, len, "IPv6");  break;
        case SNAP_TYPE_ARP:    snprintf(type_string, len, "ARP");   break;
        case SNAP_TYPE_EAP:    snprintf(type_string, len, "EAP");   break;
        default:               snprintf(type_string, len, "TYPE?"); break;
    };
 };

 void pkt_get_ether_type_string(uint8_t *data_start, uint32_t data_size, char * type_string, int len) {
    unsigned int type_start = 6;
    uint16_t     ether_type;
    uint16_t     snap_type = (data_start[type_start] << 8) + data_start[type_start+1];
    type_start += 2;
    switch (snap_type) {
        case SNAP_TYPE_IPV4:
            type_start += 9;
            break;
        case SNAP_TYPE_IPV6:
            type_start += 7;
            break;
        default:
            return;
    };
    ether_type = data_start[type_start];
    //printf("start: %d, code: %02X\n", type_start, ether_type);
    switch (ether_type) {
        case ETHER_TYPE_ICMPV4:   snprintf(type_string, len, "ICPMv4");   break;
        case ETHER_TYPE_IGMPV4:   snprintf(type_string, len, "IGMPv4");   break;
        case ETHER_TYPE_TCP:      snprintf(type_string, len, "TCP");      break;
        case ETHER_TYPE_UDP:      snprintf(type_string, len, "UDP");      break;
        case ETHER_TYPE_IPV6:     snprintf(type_string, len, "EAP");      break;
        case ETHER_TYPE_ROUTING:  snprintf(type_string, len, "ROUTING");  break;
        case ETHER_TYPE_RSVP:     snprintf(type_string, len, "RSVP");     break;
        case ETHER_TYPE_ICMPV6:   snprintf(type_string, len, "ICMPv6");   break;
        case ETHER_TYPE_NONE:     snprintf(type_string, len, "EAP");      break;
        default:                  snprintf(type_string, len, "TYPE?");    break;
    };
 }

#define MAX_SIZE 3000
pcap_dumper_t *outputFile;
pcap_t *fileHandle;
char *outputFileName = "bcaster.cap";

int pkt_open_file() {
	fileHandle = pcap_open_dead(DLT_IEEE802_11_RADIO, MAX_SIZE);
	outputFile = pcap_dump_open(fileHandle,outputFileName);
	if (!outputFile) return 0;
	else return 1;
}

void pkt_write_file(uint8_t *pkt, uint32_t pkt_len) {
    struct pcap_pkthdr header;
    struct timeval timestamp;
    gettimeofday(&timestamp, NULL);
    header.ts = timestamp;
    header.caplen = pkt_len;
    header.len = pkt_len;
	pcap_dump((u_char *) outputFile, &header, pkt);
}

void pkt_close_file() {
    pcap_close(fileHandle);
}



