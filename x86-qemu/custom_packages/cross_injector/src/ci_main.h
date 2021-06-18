/*!
 * @file src/ci_main.h
 * @brief specifies configuration options for the main application
 * @details
 * The main options to change are whether to use vsock or not and the server address (for either vsock or normal IP)
 *
 */

#ifndef MAIN_H
#define MAIN_H

#define USLEEP_TIME 500

#define USE_VSOCK 1

#ifdef USE_VSOCK
#define SERVER_ADDR "2"
#define TEST_SERVER_ADDR "2"
#else
#define SERVER_ADDR "192.168.224.101"
#define TEST_SERVER_ADDR "127.0.0.1"
#endif
#define SERVER_PORT 9991
#define BUFFER_SIZE 2500
#define TEST_FILE "test.pkt"

#define NO_ERROR             0
#define ERROR_PKT_NOT_VALID -1
#define ERROR_LIBPCAP       -2
#define ARGUMENT_ERR        -3
#define SOCKET_ERROR        -4
#define SHUTDOWN            -5
#define OK_DUPLICATE         1
#define OK_TIMEOUT           2
#define OK(x) x>=NO_ERROR

#endif
