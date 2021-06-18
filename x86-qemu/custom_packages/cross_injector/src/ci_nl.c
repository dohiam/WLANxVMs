/*!
 * @file src/ci_nl.c
 * @brief for using netlink to capture and receive
 * @details
 * This is a work in progress. It can receive packets from mac80211_hwsim using the wmediumd changes, but injecting packets via netlink is TBD.
 *
 * fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).
 */

#define BIT(x) 1 << x
#define s8 char
#define u8 unsigned char
#include "ci_nl.h"
#include "mac80211_hwsim.h"
#include <netlink/netlink.h>
#include <netlink/msg.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <netlink/genl/family.h>
#include <stdio.h>
#include <pthread.h>
#include "item.h"
#include "queue.h"
#include "radiotap.h"
#include "utilities.h"
#include "aq_type.h"

#define NUM_RADIOS 2

static aq_type * nl_capture_send_q;     /* capture is the producer, send is the consumer */
static aq_type * nl_receive_inject_q;   /* receive is the producer, inject is the consumer */

typedef struct nl_item_s {
    int size;
    uint8_t buffer[BUFFER_SIZE];
} nl_item_t;

struct mac_address {
	unsigned char addr[6];
};

static struct nl_sock *nlsock;

static int received = 0;
//static int sent = 0;
//static int dropped = 0;
//static int acked = 0;

int cinl_received() {
    return received;
}

/* put on captured frames queue  */

static void capture_frame(uint8_t *data, unsigned int data_len, unsigned int freq) {
    nl_item_t * item = (nl_item_t *) aq_get_tail(nl_capture_send_q);
    if (!item) {
        printf("nl_capture queue is full, can't capture right now\n");
        return;
    }
    rt_set_channel_header(item->buffer, freq);
    /* TODO */
    aq_put_tail(nl_capture_send_q);
    received++;
}
/* put on captured frames queue and send back to hwsim to transmit */
static void transmit_frame(uint8_t *data, unsigned int data_len, unsigned int freq,
                           unsigned long cookie, struct hwsim_tx_rate *tx_rates, struct mac_address *src) {
    capture_frame(data, data_len, freq);
    /* todo: if */

}

static void process_hwsim_cmd(struct nlattr **attrs) {
    struct mac_address *src;
    unsigned int data_len;
    uint8_t *data;
    unsigned int flags;
    struct hwsim_tx_rate *tx_rates;
    unsigned long cookie;
    unsigned int freq; //HWSIM_ATTR_FREQ
    //unsigned int rate; // HWSIM_ATTR_RX_RATE
    //signal HWSIM_ATTR_SIGNAL

    src = NULL;
    data = NULL;
    tx_rates = NULL;
    flags = 0;
    cookie = freq = 0;
    if (attrs[HWSIM_ATTR_ADDR_TRANSMITTER]) src = (struct mac_address*) nla_data(attrs[HWSIM_ATTR_ADDR_TRANSMITTER]);
    else printf("no transmitter\n");
    if (attrs[HWSIM_ATTR_FRAME]) {data_len = nla_len(attrs[HWSIM_ATTR_FRAME]); data = (uint8_t *)nla_data(attrs[HWSIM_ATTR_FRAME]);}
    else printf("no frame\n");
    if (attrs[HWSIM_ATTR_FLAGS]) flags = nla_get_u32(attrs[HWSIM_ATTR_FLAGS]);
    else printf("no flags\n");
    if (attrs[HWSIM_ATTR_TX_INFO]) tx_rates = (struct hwsim_tx_rate*) nla_data(attrs[HWSIM_ATTR_TX_INFO]);
    else printf("no rates\n");
    if (attrs[HWSIM_ATTR_COOKIE]) cookie = nla_get_u64(attrs[HWSIM_ATTR_COOKIE]);
    else printf("no cookie\n");
    if (attrs[HWSIM_ATTR_FREQ]) freq = nla_get_u32(attrs[HWSIM_ATTR_FREQ]);
    else printf("no freq\n");
    //q2print_hex(data, data_len);
    if (!flags) flags = 0;
    transmit_frame(data, data_len, freq, cookie, tx_rates, src);
}

 /***************************
  * netlink callback function
  ***************************/

static int nlcb_function (struct nl_msg *nlmsg, void *arg)
{
    static struct nlattr      *nlattrs[HWSIM_ATTR_MAX+1];
    static struct nlmsghdr    *nlmsgheader;
    static struct genlmsghdr  *gnlmsgheader;

	nlmsgheader = nlmsg_hdr(nlmsg);
	if (!nlmsgheader) return -1;
	gnlmsgheader = (struct genlmsghdr *) nlmsg_data(nlmsgheader);
	if (!gnlmsgheader) return -1;

	switch (gnlmsgheader->cmd) {
        case HWSIM_CMD_FRAME:
            genlmsg_parse(nlmsgheader, 0, nlattrs, HWSIM_ATTR_MAX, NULL);
            process_hwsim_cmd(nlattrs);
            break;
       	case HWSIM_CMD_UNSPEC:          printf("received nlmsg: HWSIM_CMD_UNSPEC\n"); break;
        case HWSIM_CMD_REGISTER:        printf("received nlmsg: HWSIM_CMD_REGISTER\n"); break;
        case HWSIM_CMD_TX_INFO_FRAME:   printf("received nlmsg: HWSIM_CMD_TX_INFO_FRAME\n"); break;
        case HWSIM_CMD_NEW_RADIO:       printf("received nlmsg: HWSIM_CMD_NEW_RADIO\n"); break;
        case HWSIM_CMD_DEL_RADIO:       printf("received nlmsg: HWSIM_CMD_DEL_RADIO\n"); break;
        case HWSIM_CMD_GET_RADIO:       printf("received nlmsg: HWSIM_CMD_GET_RADIO\n"); break;
        default:                        printf("received nlmsg: default\n"); break;
	}

	return 0;

}


int cinl_init(struct queue_t *nlcaptureq)
{
    int rc;
    struct nl_msg      *nlmsg;
    struct nl_cb       *nlcb;
    struct nl_cache    *nlcache;
    struct genl_family *gnlfamily;

    if (!aq_new(&nl_capture_send_q, sizeof(nl_item_t), NUM_ITEMS)) {
        printf("error allocating nl_capture_send_q\n");
        return -1;
    };
    if (!aq_new(&nl_receive_inject_q, sizeof(nl_item_t), NUM_ITEMS)) {
        printf("error allocating nl_receive_inject_q\n");
        return -1;
    };

	nlcb = nl_cb_alloc(NL_CB_CUSTOM);
	if (!nlcb) {
	    printf("error allocating netlink callback\n");
        return 0;
	}

	nlsock = nl_socket_alloc_cb(nlcb);
	if (!nlsock) {
		printf("error allocating netlink socket\n");
		return 0;
	}

	rc = genl_connect(nlsock);
	if (rc<0) {
        printf("error connecting netlink socket\n");
        return 0;
	}

	rc = genl_ctrl_alloc_cache(nlsock, &nlcache);
	if (rc <0) {
        printf("error allocating netlink control cache\n");
        return 0;
	}

	gnlfamily = genl_ctrl_search_by_name(nlcache, HWSIM_FAMILY_NAME);
	if (!gnlfamily) {
		printf("error searching for hwsim family name\n");
		return 0;
	}

	rc = nl_cb_set(nlcb, NL_CB_MSG_IN, NL_CB_CUSTOM, nlcb_function, NULL);
	if (rc <0) {
        printf("error setting netlink callback function\n");
        return 0;
	}

    nlmsg = nlmsg_alloc();
	if (!nlmsg) {
		printf("Error allocating new message MSG!\n");
		return 0;
	}
	genlmsg_put(nlmsg, NL_AUTO_PID, NL_AUTO_SEQ, genl_family_get_id(gnlfamily), 0, NLM_F_REQUEST, HWSIM_CMD_REGISTER, HWSIM_NL_VERSION);
	nl_send_auto(nlsock, nlmsg);
	nlmsg_free(nlmsg);

	rc = nl_socket_set_nonblocking(nlsock);
	if (!rc) printf("error setting netlink socket to non-blocking");

	return 1;

}

void cinl_run() {
    nl_recvmsgs_default(nlsock);
}

#define SIZE_OF_ACK RT_CHANNEL_HEADER_SIZE + 10

/* todo: what was this for?
static uint8_t * ack_to(struct mac_address *src, uint16_t freq) {
    uint8_t *ack_pkt;
    int i, j;
    ack_pkt = malloc(SIZE_OF_ACK);
    if (!ack_pkt) {printf("Error allocating ack_pkt\n"); return NULL;}
    rt_set_channel_header(ack_pkt, freq);
    ack_pkt[RT_CHANNEL_HEADER_SIZE+0] = 0xD4;   // type-subtype-version
    ack_pkt[RT_CHANNEL_HEADER_SIZE+1] = 0;      // flags
    ack_pkt[RT_CHANNEL_HEADER_SIZE+2] = 0;      // duration id
    ack_pkt[RT_CHANNEL_HEADER_SIZE+3] = 0;      // duration id
    for (i=RT_CHANNEL_HEADER_SIZE+4, j=0; i<RT_CHANNEL_HEADER_SIZE+4+6; i++, j++) ack_pkt[i] = src->addr[j];
    return ack_pkt;
}
*/




