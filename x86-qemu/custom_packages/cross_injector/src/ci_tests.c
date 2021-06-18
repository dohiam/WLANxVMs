/*!
 * @file src/ci_tests.c
 * @brief implements several unit tests to test various parts of the application
 *
 * fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).
 */

#define QUEUES_TEST

#include "looper.h"
#include "item.h"
#include "queue.h"
#include "ci_queues.h"
#include "ci_workers.h"
#include "ci_client_server.h"
#include "utilities.h"
#include "radiotap.h"
#include "ci_main.h"
#include "lisa.h"
#include <assert.h>
#include <stdio.h>
#include <pcap.h>

/**************************************
    Unit Tests
 **************************************/

static  unsigned char beacon1[] = {
	0x00,0x00,0x0b,0x00,0x04,0x0C,0x00,0x00,0x6C,0x0C,0x01,0x80,0x00,0x00,0x00,0xff,0xff,0xff,0xff,0xff,0xff,0x02,0x00,0x00,0x00,0x00,0x00,0x02,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x8a,0x1d,0x0b,0x51,0xd7,0xa7,0x05,0x00,0xf4,0x01,0x01,0x04,0x00,0x06,0x74,0x65,0x73,0x74,0x61,0x70,0x01,0x08,0x82,0x84,0x8b,0x96,0x0c,0x12,0x18,0x24,0x03,0x01,0x06,0x05,0x04,0x01,0x02,0x00,0x00,0x2a,0x01,0x04,0x32,0x04,0x30,0x48,0x60,0x6c,0x3b,0x02,0x51,0x00,0x7f,0x08,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x40
};
static  unsigned int beacon1_len = 97;

static  unsigned char beacon2[] = {
	0x00,0x00,0x0b,0x00,0x04,0x0C,0x00,0x00,0x6C,0x0C,0x01,0x80,0x00,0x00,0x00,0xff,0xff,0xff,0xff,0xff,0xff,0x02,0x00,0x00,0x00,0x00,0x00,0x02,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x79,0x0d,0x61,0x51,0xd7,0xa7,0x05,0x00,0xf4,0x01,0x01,0x04,0x00,0x06,0x74,0x65,0x73,0x74,0x61,0x70,0x01,0x08,0x82,0x84,0x8b,0x96,0x0c,0x12,0x18,0x24,0x03,0x01,0x06,0x05,0x04,0x00,0x02,0x00,0x00,0x2a,0x01,0x04,0x32,0x04,0x30,0x48,0x60,0x6c,0x3b,0x02,0x51,0x00,0x7f,0x08,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x40
};
static  unsigned int beacon2_len = 97;

static  unsigned char beacon3[] = {
	0x00,0x00,0x0b,0x00,0x04,0x0C,0x00,0x00,0x6C,0x0C,0x01,0x80,0x00,0x00,0x00,0xff,0xff,0xff,0xff,0xff,0xff,0x02,0x00,0x00,0x00,0x00,0x00,0x02,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x6d,0x8d,0x9f,0x51,0xd7,0xa7,0x05,0x00,0xf4,0x01,0x01,0x04,0x00,0x06,0x74,0x65,0x73,0x74,0x61,0x70,0x01,0x08,0x82,0x84,0x8b,0x96,0x0c,0x12,0x18,0x24,0x03,0x01,0x06,0x05,0x04,0x00,0x02,0x00,0x00,0x2a,0x01,0x04,0x32,0x04,0x30,0x48,0x60,0x6c,0x3b,0x02,0x51,0x00,0x7f,0x08,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x40
};
static  unsigned int beacon3_len = 97;


static lisa l_server;
static lisa *lp_server=&l_server;

static lisa l_client;
static lisa *lp_client=&l_client;

static pcap_t *pcap_capture_handle = NULL;
static pcap_t *pcap_injection_handle = NULL;


/**************************************
    looper
 **************************************/

static int test_data;
static int test_final;

void test_init_function(void *data) {
    int *ip = (int *) data;
    *ip = 0;
    test_final = 0;
}

void test_looper_function(void *data) {
    int *ip = (int *) data;
    *ip = *ip + 1;
}

void test_final_function(void *data) {
    int *ip = (int *) data;
    test_final = *ip;
}

void test_looper() {
    int test_data_last;
    struct looper_t l;
    looper_init(&l);
    l.function_to_run_first = test_init_function;
    l.function_to_repeat = test_looper_function;
    l.function_to_run_last = test_final_function;
    l.data = (void *) &test_data;
    assert(test_data == 0);
    assert(test_final == 0);
    looper_start(&l);
    usleep(LOOPER_SLEEP_TIME_DEFAULT*10);
    pthread_mutex_lock(&(l.lock));
    q2print("test_data: %d\n", test_data);
    assert(0 < test_data);
    test_data_last = test_data;
    assert(test_final == 0);
    pthread_mutex_unlock(&(l.lock));
    looper_pause(&l);
    usleep(LOOPER_SLEEP_TIME_DEFAULT*10);
    pthread_mutex_lock(&(l.lock));
    q2print("test_data: %d\n", test_data);
    assert((test_data_last == test_data) || (test_data_last == test_data - 1));
    test_data_last = test_data;
    pthread_mutex_unlock(&(l.lock));
    looper_resume(&l);
    usleep(LOOPER_SLEEP_TIME_DEFAULT*10);
    pthread_mutex_lock(&(l.lock));
    assert(test_data_last < test_data);
    pthread_mutex_unlock(&(l.lock));
    looper_stop(&l);
    q2print("test_data: %d\n", test_data);
    q2print("test_final: %d\n", test_final);
    assert(test_data == test_final);
    q2print("looper test passes\n");
}

/**************************************
   item
 **************************************/

static void test_item() {
    struct item_t *item1, *item2, *item3, *item_a, *item_b, *item_c;
    q2print("testing making and freeing an item\n");
    item1 = item_make(beacon1, beacon1_len);
    assert(item1);
    assert(item1->buffer);
    for (int i=0; i<item1->size; i++) assert(item1->buffer[i] == beacon1[i]);
    assert(item1->next_item == NULL);
    item_free(&item1);
    assert(!item1);
    q2print("making and freeing test pass\n");
    q2print("testing item copy and match\n");
    item1 = item_make(beacon1, beacon1_len);
    item2 = item_make(beacon2, beacon2_len);
    item3 = item_make(beacon3, beacon3_len);
    item_copy(item1, &item_a);
    item_copy(item2, &item_b);
    item_copy(item3, &item_c);
    assert(item_match(item1, item_a));
    assert(item_match(item2, item_b));
    assert(item_match(item3, item_c));
    item_free(&item1);
    item_free(&item2);
    item_free(&item3);
    item_free(&item_a);
    item_free(&item_b);
    item_free(&item_c);
    q2print("copy and match test pass\n");
}

static void test_queue() {
    struct item_t *item1, *item2, *item3, *item_a, *item_b, *item_c;
    struct queue_t q1, q2;
    q2print("testing queue functions\n");
    item1 = item_make(beacon1, beacon1_len);
    item2 = item_make(beacon2, beacon2_len);
    item3 = item_make(beacon3, beacon3_len);
    q_init(&q1);
    q_init(&q2);
    /* put */
    q_put_item(item1, &q1);
    assert(q1.current_item == item1);
    q_put_item(item2, &q1);
    assert(q1.current_item == item1);
    assert(q1.last_item == item2);
    assert(q1.current_item->next_item == item2);
    q_put_item(item3, &q1);
    assert(q1.last_item == item3);
    /* get */
    item_a = q_get_item(&q1);
    assert(item_a == item1);
    assert(q1.current_item == item2);
    item_b = q_get_item(&q1);
    assert(item_b == item2);
    assert(q1.current_item == item3);
    item_c = q_get_item(&q1);
    assert(item_c == item3);
    assert(q1.current_item == NULL);
    q_put_item(item_a, &q1);
    q_put_item(item_b, &q1);
    q_put_item(item_c, &q1);
    item1 = item2 = item3 = NULL;
    item1 = q_get_item(&q2);
    assert(item1 == NULL);
    q_move_item(&q1, &q2);
    q_move_item(&q1, &q2);
    q_move_item(&q1, &q2);
    item1 = q_get_item(&q2);
    item2 = q_get_item(&q2);
    item3 = q_get_item(&q2);
    assert(item_a == item1);
    assert(item_b == item2);
    assert(item_c == item3);
    item_free(&item1);
    item_free(&item2);
    item_free(&item3);
    q2print("queue put and get test pass\n");
}


void test_capture2send() {
    struct item_t *item1, *item2, *item3, *item_a, *item_b, *item_c;
    q2print("testing capture2send\n");
    q2print("starting queues (paused)\n");
    ciqs_init_queues();
    ciws_init(NULL); //don't need the listen worker
    ciws_start(capture2altsendw);
    ciws_pause(capture2sendw);
    q2print("making and putting items in captureq\n");
    item1 = item_make(beacon1, beacon1_len);
    item2 = item_make(beacon2, beacon2_len);
    item3 = item_make(beacon3, beacon3_len);
    assert(ciqs_get_item_ts(sendq) == NULL);
    ciqs_put_item_ts(item1, captureq);
    ciqs_put_item_ts(item2, captureq);
    ciqs_put_item_ts(item3, captureq);
    q2print("resuming capture2send\n");
    ciws_resume(capture2sendw);
    sleep(1);
    q2print("stopping capture2send\n");
    ciws_stop(capture2sendw);
    q2print("getting and comparing items\n");
    item_a = ciqs_get_item_ts(sendq);
    item_b = ciqs_get_item_ts(sendq);
    item_c = ciqs_get_item_ts(sendq);
    assert(item1 == item_a);
    assert(item2 == item_b);
    assert(item3 == item_c);
    item_free(&item1);
    item_free(&item2);
    item_free(&item3);
    q2print("capture2send test pass\n");
}

void test_receive2inject() {
    struct item_t *item1, *item2, *item3, *item_a, *item_b, *item_c;
    q2print("testing receive2inject\n");
    q2print("starting queues (paused)\n");
    ciqs_init_queues();
    ciws_init(NULL); //don't need the listen worker
    ciws_start(receive2injectw);
    ciws_pause(receive2injectw);
    item1 = item_make(beacon1, beacon1_len);
    item2 = item_make(beacon2, beacon2_len);
    item3 = item_make(beacon3, beacon3_len);
    assert(ciqs_get_item_ts(injectionq) == NULL);
    ciqs_put_item_ts(item1, receiveq);
    ciqs_put_item_ts(item2, receiveq);
    ciqs_put_item_ts(item3, receiveq);
    q2print("resuming receive2inject\n");
    ciws_resume(receive2injectw);
    sleep(1);
    q2print("stopping receive2inject\n");
    ciws_stop(receive2injectw);
    q2print("getting and comparing items\n");
    item_a = ciqs_get_item_ts(injectionq);
    item_b = ciqs_get_item_ts(injectionq);
    item_c = ciqs_get_item_ts(injectionq);
    assert(item1 == item_a);
    assert(item2 == item_b);
    assert(item3 == item_c);
    item_free(&item1);
    item_free(&item2);
    item_free(&item3);
    q2print("receive2inject test pass\n");
}

void test_radiotap() {
    struct rt_data_t data;
    rt_header hdr;
    data.flags = 0;
    data.rate = 108;
    data.antenna = 1;
    data.frequency = 2437;
    data.channel_flags = 0;
    data.tsft_time = 0;
    rt_make_header_from_data(&data, hdr);
    rt_print_hdr(hdr);
}

struct queue_t test_q;

void test_all() {
    test_looper();
    test_item();
    test_queue(&test_q);
    test_send_receive_queues();
    test_capture();
    test_injection();
    q2print("all done\n");
    test_capture2send();
    test_receive2inject();
    test_radiotap();
    q2print("all tests done\n");
}

/* for testing client/server across VMs ( sends beacon1, server sends beacon2) */

void test_server(int use_vsock) {
    struct item_t *item_send, *item_receive;
    uint64_t expected_hash, actual_hash;
    q2print("starting queues\n");
    ciqs_init_queues();
    ciqs_start(altsendq, lp_server);
    ciqs_start(altreceiveq, lp_server);
    ciws_init(lp_server);  // initialize listener
    q2print("starting server\n");
    ci_start_server(SERVER_PORT, lp_server, use_vsock);
    /* send beacon 2 and compare received to hash of beacon 1 */
    item_send = item_make(beacon2, beacon2_len);
    item_receive = item_make(beacon1, beacon1_len);
    expected_hash = rt_get_payload_hash(item_receive);
    item_free(&item_receive);
    ciqs_put_item_ts(item_send, altsendq);
    q2print("waiting to have received something\n");
    while (!ciqs_peek_item_ts(altreceiveq)) sleep(1);
    item_receive = ciqs_get_item_ts(altreceiveq);
    actual_hash = rt_get_payload_hash(item_receive);
    assert (expected_hash == actual_hash);
    q2print("server test pass \n");
}

void test_client1(int use_vsock) {
    struct item_t *item_send, *item_receive;
    uint64_t expected_hash, actual_hash;
    q2print("starting client\n");
    ci_start_client(TEST_SERVER_ADDR, SERVER_PORT, lp_client, use_vsock);
    q2print("starting queues\n");
    ciqs_init_queues();
    ciqs_start(sendq, lp_client);
    ciqs_start(receiveq, lp_client);
    sleep(10);
    /* send beacon 1 and compare received to hash of beacon 2 */
    item_send = item_make(beacon1, beacon1_len);
    item_receive = item_make(beacon2, beacon2_len);
    expected_hash = rt_get_payload_hash(item_receive);
    item_free(&item_receive);
    ciqs_put_item_ts(item_send, sendq);
    q2print("waiting to have received something\n");
    while (!ciqs_peek_item_ts(receiveq)) sleep(1);
    item_receive = ciqs_get_item_ts(receiveq);
    actual_hash = rt_get_payload_hash(item_receive);
    assert (expected_hash == actual_hash);
    q2print("client test pass \n");
}

void test_client2(int use_vsock) {
    struct item_t *item_send, *item_receive;
    uint64_t expected_hash, actual_hash;
    q2print("starting client\n");
    ci_start_client(TEST_SERVER_ADDR, SERVER_PORT, lp_client, use_vsock);
    q2print("starting queues\n");
    ciqs_init_queues();
    ciqs_start(sendq, lp_client);
    ciqs_start(receiveq, lp_client);
    sleep(10);
    /* send beacon 2 and compare received to hash of beacon 1 */
    item_send = item_make(beacon2, beacon2_len);
    item_receive = item_make(beacon1, beacon1_len);
    expected_hash = rt_get_payload_hash(item_receive);
    item_free(&item_receive);
    ciqs_put_item_ts(item_send, sendq);
    q2print("waiting to have received something\n");
    while (!ciqs_peek_item_ts(receiveq)) sleep(1);
    item_receive = ciqs_get_item_ts(receiveq);
    actual_hash = rt_get_payload_hash(item_receive);
    assert (expected_hash == actual_hash);
    q2print("client test pass \n");
}

void test_fs_malloc() {
    unsigned char * ptr, * ptr1, * ptr2, * ptr3, * ptr4, * ptr5;
    int i;
    ptr = fs_malloc(0);
    assert(ptr == NULL);
    fs_malloc_open();
    for(i=0; i<FS_MAX_NUM; i++) {
      ptr = (unsigned char *) fs_malloc(0);
      assert(ptr != NULL);
    }
    ptr = fs_malloc(0);
    assert(ptr == NULL);
    fs_malloc_open();
    for (i=0; i<FS_MAX_NUM*3; i++) {
        ptr1 = (unsigned char *) fs_malloc(i);
        ptr2 = (unsigned char *) fs_malloc(i);
        ptr3 = (unsigned char *) fs_malloc(i);
        ptr4 = (unsigned char *) fs_malloc(i);
        ptr5 = (unsigned char *) fs_malloc(i);
        assert(ptr1!=NULL);
        assert(ptr2!=NULL);
        assert(ptr3!=NULL);
        assert(ptr4!=NULL);
        assert(ptr5!=NULL);
        fs_free(ptr1);
        fs_free(ptr2);
        fs_free(ptr3);
        fs_free(ptr4);
        fs_free(ptr5);
    }
    fs_malloc_close();
    printf("fs_malloc test pass \n");
}

