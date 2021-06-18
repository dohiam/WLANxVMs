/*!
 * @file src/ci_client_server.h
 * @brief starts the sending & receiving socket
 * @details
 * The client part of this is the only part needed for production. The server is used for testing (to test the client).
 *
 * fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).
 */

#ifndef CLIENT_SERVER_H
#define CLIENT_SERVER_H

#define STOP_MSG "***STOP***NOW***"
#define STOP_MSG_LEN sizeof(STOP_MSG)

/* Start Sharing: either by starting the client or server
 * client & server are threads that share packets between VMs; they maintain a queue of received packets and packets to send
 * Generally will run AP and STAs in different VMs (recommend you put the server in the VM that is the access point, and start clients in each STA)
 * but you could run clients & server in same VM using localhost (which is why client/server is used instead of multicast).
 */

#include "lisa.h"

#define CI_NO_ERROR 0
#define CI_SOCKET_ERROR -1

int ci_start_client(char * server_addr, int port, lisa *lp, int use_vsock);

int ci_start_server(int port, lisa *lp, int use_vsock);

/* these affect all threads/workers associated with client/server */

void ci_pause_client();
void ci_resume_client();
void ci_stop_client();

void ci_pause_server();
void ci_resume_server();
void ci_stop_server();


#endif // CLIENT_SERVER_H
