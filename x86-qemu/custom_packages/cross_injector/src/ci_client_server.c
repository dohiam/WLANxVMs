/*!
 * @file src/ci_client_server.c
 * @brief starts the sending & receiving socket
 * @details
 * Opns the socket and connects to the server specified by the address. If use_vsock is passed as zero then the server address is
 * used as a normal IP address.
 *
 * fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).
 */

#include "ci_client_server.h"
#include "ci_queues.h"
#include "ci_workers.h"
#include "lisa.h"
#include <stdio.h>
#include <sys/ioctl.h>
//#include "radiotap.h"
#include "utilities.h"

int ci_start_client(char * server_addr, int port, lisa *lp, int use_vsock) {
    lisa_open(lp);
    q2print("using vsock: %d\n", use_vsock);
    if (use_vsock) lisa_set_to_vsock(lp);
    lisa_setup_tcp(lp);
    lisa_set_wait_time(lp, 2);
    q2print("client connecting...\n");
    lisa_connect(lp, server_addr, port);
    q2print("lp fd: %d\n", lp->fd);
    if (lp->state != LISA_CONNECTED) {
        q2print("Did not connect to server\n");
        q2print("%s\n", lp->debugmsg);
        return CI_SOCKET_ERROR;
    }
    q2print("client ready\n");
    return CI_NO_ERROR;
}

int ci_start_server(int port, lisa *lp, int use_vsock) {
    lisa_open(lp);
    if (use_vsock) lisa_set_to_vsock(lp);
    lisa_setup_tcp(lp);
    lp->port = port;
    lisa_set_wait_time(lp, 2);
    q2print("server ready\n");
    return CI_NO_ERROR;
}

void ci_pause_client() {
    ciqs_pause(sendq);
    ciqs_pause(receiveq);
}

void ci_resume_client() {
    ciqs_resume(sendq);
    ciqs_resume(receiveq);
}

void ci_stop_client(lisa *lp) {
    ciqs_stop(sendq);
    ciqs_stop(receiveq);
    lisa_send(lp, STOP_MSG, STOP_MSG_LEN);
}

void ci_pause_server() {
    ciqs_pause(altsendq);
    ciqs_pause(altreceiveq);
}

void ci_resume_server() {
    ciqs_resume(altsendq);
    ciqs_resume(altreceiveq);
}

void ci_stop_server() {
    ciqs_stop(altsendq);
    ciqs_stop(altreceiveq);
}


