/*!
 * @file src/ci_tests.h
 * @brief defines several unit tests to test various parts of the application
 * @details
 * These would be called by /ref src/ci_main.c as needed to run the tests.
 *
 * fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).
 */

#ifndef CI_TESTS
#define CI_TESTS

#include "queue.h"

void test_looper();

void test_item();

void test_copy_match();

void test_put_get(struct queue_t *q, char *qname);

void test_client_server();

//ensure AP is running

void test_capture();

void test_injection();

void test_capture2send();

void test_receive2inject();

void test_all();

void test_server(int use_vsock);
void test_client1(int use_vsock);
void test_client2(int use_vsock);

void test_fs_malloc();

#endif // CI_TESTS
