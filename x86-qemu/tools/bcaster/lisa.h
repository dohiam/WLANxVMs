/**
 * \file
 * Lightweight Ip-Socket Abstraction
 *
 * David Hamilton - 2007
 *
 * To show you how really easy this makes everything, here are basic examples:
 *
 * TCP SERVER:
 *   lisa_start();
 *   lisa_open(l);
 *   lisa_accept(l, HOST_PORT);
 *   lisa_recv(l, data, 80);
 *   lisa_send(l, msg, strlen(msg));
 *   printf("RECEIVED: %s", data);
 *   lisa_close(l);
 *   lisa_stop();
 *
 * TCP CLIENT:
 *   tube_connect(t, HOST_ADDR, HOST_PORT);
 *   tube_send_to(t, msg, strlen(msg));
 *   tube_recv(t, data, 80);
 *   printf("RECEIVED: %s", data);
 *   tube_close(t);
 *
 * UDP NON-BLOCKING SERVER:
 *   tube_setup_udp(t);
 *   tube_set_nonblocking(t);
 *   tube_make_quiet();
 *   for (i = 0; i< 60; i++) {
 *     bytes_received = tube_recv_from(t, data, 80, HOST_PORT);
 *     if (bytes_received > 0) break;
 *     printf("*");
 *     Sleep(1000);
 *   }
 *   printf("\n");
 *   tube_send(t, msg, strlen(msg));
 *   printf("RECEIVED: %s", data);
 *   tube_close(t);
 *
 * UDP CLIENT:
 *   tube_send_to(t, msg,strlen(msg), HOST_ADDR, HOST_PORT);
 *   tube_recv(t, data, 80);
 *   printf("RECEIVED: %s", data);
 *   tube_close(t);
 *
 * Notes
 *  1) Normally it is not necessary to call setup_tcp or setup_udp. The first call to send, send_to, listen, etc.
 *     indicates whether a socket is going to be treated as tcp or udp. So under the covers, a socket is automatically
 *     set up as tcp or udp depending on how it is being used.
 *  2) The non-blocking server does call setup_udp because the subsequent call to make it nonblocking can work
 *     for either tcp or udp sockets so it doesn't know which one to initialize it for, and therefore we need to
 *     explicitly set it to udp.
 *  3) reads are guarded with a select statment with a default timeout of one second. If you want to chang the
 *     timeout, set tube->wait_time.tv_sec to the number of seconds you wan to wait (or tube->wait_time_tv_usec);
 *  4) by default, sockets are nonblocking ("by default" means if you call connect without calling setup_tcp).
 *  5) Servers can have up to MAX_CLIENTS connected at a time. Sends will go to whichever client the last recv
 *     came from. The expectation is that servers will be in a loop receiving and then optionally responding to
 *     the client that most recently sent something. So, for example, trying to receive from three different clients
 *     and then responding to all 3 will result in all 3 responses going to the last client who sent something.
 *
 */

#ifndef LISA_H
#define LISA_H

#define MAX_CLIENTS 10

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <pthread.h>
#else
#define SOCKET int
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <linux/vm_sockets.h>
#endif

/** \brief
 * This will use either strerror or WSAGetLastError depending on the platform.
 * (for Windows, there are hard coded strings for errors that pertain to sockets)
 * If msg is NULL then it just returns the error number and you must use the
 * appropriate platform specific enum to compare.
 */
int lisa_get_last_error(char * msg, int msg_length);

typedef struct timeval stdtimeval;

#define DEBUGMSGLEN 80

/** \brief
 * This is the main datastructure. Pass a pointer to one of these
 * to each tube function
 * local_addr and remote_addr are the real socket data structures
 * for this side and the other side of the socket.
 */
struct lisa_def {
  struct sockaddr_in   local_addr;   /* local protocol, address, port */
  struct sockaddr_in   remote_addr;  /* remote protocol, address, port */
  struct sockaddr_vm   local_vaddr;  /* local protocol, address, port */
  struct sockaddr_vm   remote_vaddr; /* remote protocol, address, port */
  stdtimeval           wait_time;    /* how long to wait on reads (using select) */
  SOCKET               fd;           /* the socket, or for a server the listening socket */
  SOCKET               client_fd;    /* for a server this is the most recent accept */
  SOCKET               client_fds[MAX_CLIENTS];   /* for a server this is what accept returns for each client connecting */
  SOCKET               this_fd;      /* fd from most recent select */
  int                  num_clients;  /* number of clients with a connection */
  int                  state;        /* used to short circuit if tube is in a bad state */
  int                  client_state; /* for a server, the state of the connection with a client */
  int                  flags;        /* used to customize tube behavior */
  int                  timeout;      /* for things like opening a connection */
  int                  port;         /* port to use */
  char                 type;         /* tcp, udp */
  char                 clsvr;        /* client or server */
  char                 vsock;        /* inet or vsock connection type: default inet (i.e., vsock=0) */
  char                 blocking;     /* 0 = nonblocking */
  //fd_set               fdset;        /* for select on reads */
  char                 debugmsg[DEBUGMSGLEN]; /* the last error or last status */
  pthread_mutex_t      lock;

};
typedef struct lisa_def lisa;

/* macros to add file and line automatically as strings to each call for auto-error tracking */

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define AT __FILE__ ":" TOSTRING(__LINE__)


#ifdef DEBUG
#define BREADCRUMB TOSTRING(__FILE__), TOSTRING(__LINE__)
#else
#define BREADCRUMB
#endif

#define ANY_ADDRESS htonl(INADDR_ANY)

/** \brief
 * if win32 then start/stop winsock; otherwise doesn't do anything
 * (just do once at the beginning and end of an application, do not
 *  start and stop for each connection)
 * Also, note that the start will be called automatically if you forget.
 */
#ifdef DEBUG
#define lisa_start() LISA_START(BREADCRUMB)
#define lisa_stop() LISA_STOP(BREADCRUMB)
void LISA_START(char * file, char * line);
void LISA_STOP(char * file, char * line);
#else
void lisa_start();
void lisa_stop();
#endif

/** \brief
 * initialize and uninitialize a lisa
 *
 */
#ifdef DEBUG
#define lisa_open(l) LISA_OPEN(l, BREADCRUMB)
#define lisa_close(l) LISA_CLOSE(l, BREADCRUMB)
void LISA_OPEN(lisa* l, char * file, char * line);
void LISA_CLOSE(lisa *l, char * file, char * line);
#else
void lisa_open(lisa* l);
void lisa_close(lisa *l);
#endif

/** \brief
 * initialize a tube to be a tcp socket, don't yet say if it is a client or server
 * Note that you generally don't have to call this. The first calls to connect,
 * receive, etc. will create the socket as the appropriate type.
 */
#ifdef DEBUG
#define lisa_setup_tcp(l) LISA_SETUP_TCP(l, BREADCRUMB)
void LISA_SETUP_TCP(lisa *l, char * file, char * line);
#else
void lisa_setup_tcp(lisa *l);
#endif

/** \brief
 * initialize a tube to be a udp socket; again, doesn't matter if client or server
 * Note that you generally don't have to call this. The first calls to connect,
 * receive, etc. will create the socket as the appropriate type.
 */
#ifdef DEBUG
#define lisa_setup_udp(l) LISA_SETUP_UDP(l, BREADCRUMB)
void LISA_SETUP_UDP(lisa *l, char * file, char * line);
#else
void lisa_setup_udp(lisa *l);
#endif

/** \brief
 * called in a client to connect to a host
 */
#ifdef DEBUG
#define lisa_connect(l, addr, port) LISA_CONNECT(l, addr, port, BREADCRUMB)
void LISA_CONNECT(lisa *l, char * addr, unsigned short port, char * file, char * line);
#else
void lisa_connect(lisa *l, char * addr, unsigned short port);
#endif

/** \brief
 * called in a client to connect to a host using a name (not an ip address)
 */
#ifdef DEBUG
#define lisa_connect_by_name(l, host_name, port) LISA_CONNECT_BY_NAME(l, host_name, port, BREADCRUMB)
void LISA_CONNECT_BY_NAME(lisa *l, char * host_name, unsigned short port, char * file, char * line);
#else
void lisa_connect_by_name(lisa *l, char * host_name, unsigned short port);
#endif

/** \brief
 * called in a server to start listening for incoming connections
 * This works for TCP or UDP clients.
 */
#ifdef DEBUG
#define lisa_listen(l, port) LISA_LISTEN(l, port, BREADCRUMB)
void LISA_LISTEN(lisa *l, unsigned short port, char * file, char * line);
#else
void lisa_listen(lisa *l, unsigned short port);
#endif

/** \brief
 * called in a server to accept an incoming connecton, waiting for the
 * time out in wait_time; returns zero if timeout hit, else returns 1
 * Note that it is not necessary to call listen before accept; if you just
 * call accept, it will automatically start listening first
 */
#ifdef DEBUG
#define lisa_accept(l, port) LISA_ACCEPT(l, port, BREADCRUMB)
int LISA_ACCEPT(lisa *l, unsigned short port, char * file, char * line);
#else
int lisa_accept(lisa *l, unsigned short port);
#endif

/** \brief
 * used in a tcp client or server to send a message to the other side
 * What makes this a tcp function is that the destination is not specified;
 * this is because the destination was specified when the connection was made.
 */
#ifdef DEBUG
#define lisa_send(l, msg, msglength) LISA_SEND(l, msg, msglength, BREADCRUMB)
int  LISA_SEND(lisa *l, char *msg, unsigned int msglength, char * file, char * line);
#else
int  lisa_send(lisa *l, char *msg, unsigned int msglength);
#endif

/** \brief
 * send completely to all clients
 * send completely to all clients
 */
#ifdef DEBUG
#define lisa_cast(l, msg, msglength) LISA_CAST(l, msg, msglength, BREADCRUMB)
int  LISA_CAST(lisa *l, char *msg, unsigned int msglength, char * file, char * line);
#else
int  lisa_cast(lisa *l, char *msg, unsigned int msglength);
#endif

/** \brief
 * used to set the port to send from (uses bind).
 * If you don't care what specific port you are sending data from,
 * then there is no need to use this.
 */
#ifdef DEBUG
#define lisa_send_from(l, port) LISA_SEND_FROM(l, port, BREADCRUMB)
void  LISA_SEND_FROM(lisa *l, unsigned short port, char * file, char * line);
#else
void  lisa_send_from(lisa *l, unsigned short port);
#endif

/** \brief
 * udp version used to send a datagram to the other side
 * What makes this udp is that since no connection was previously made,
 * the destination needs to be specified here.
 */
#ifdef DEBUG
#define lisa_send_to(l, msg, msglength, addr, port) LISA_SEND_TO(l, msg, msglength, add, port, BREADCRUMB)
int  LISA_SEND_TO(lisa *l, char *msg, unsigned int msglength, char *addr, unsigned short port, char * file, char * line);
#else
int  lisa_send_to(lisa *l, char *msg, unsigned int msglength, char *addr, unsigned short port);
#endif

/** \brief
 * Receive some data over tcp. No receiving part specified because this came from
 * either the connection call or the accept call (depending on whether it is a
 * client or server). Note that whether this blocks or not
 * depends on if set_nonblocking has been called.
 * note: part argument is intended for servers who use different receive buffers for
 * different clients; it allows the server to first get the client fd and then do the receive
 * part argument == 1 means do the select part and stop
 * part argument == 2 means skip to the receive because the select has already been done
 * part argument == 12 (or anything other than 1 or 2) means do both parts 1 and 2
 * lisa_recv calls lisa_recv_with_part 12 (so the "default" is to do both)
 * note that you must do part 2 after part 1 without doing any other recv in between (on the same socket)
 *
 */
#ifdef DEBUG
#define lisa_recv(l, msg, msglength) LISA_RECV(l, msg, msglength, BREADCRUMB)
int  LISA_RECV(lisa *l, char *msg, unsigned int msglength, char * file, char * line);
#else
int  lisa_recv(lisa *l, char *msg, unsigned int msglength);
#endif

#ifdef DEBUG
#define lisa_recv_part1(l) LISA_RECV_PART1(l, BREADCRUMB)
int  LISA_RECV_PART1(lisa *l, char * file, char * line);
#else
int  lisa_recv_part1(lisa *l);
#endif

#ifdef DEBUG
#define lisa_recv_part2(l, msg, msglength) LISA_RECV_PART2(l, msg, msglength, BREADCRUMB)
int  LISA_RECV_part2(lisa *l, char *msg, unsigned int msglength, char * file, char * line);
#else
int  lisa_recv_part2(lisa *l, char *msg, unsigned int msglength);
#endif

#ifdef DEBUG
#define lisa_recv_with_part(l, msg, msglength) LISA_RECV_WITH_PART(l, msg, msglength, int part, BREADCRUMB)
int  LISA_RECV_WITH_PART(lisa *l, char *msg, unsigned int msglength, int part, char * file, char * line);
#else
int  lisa_recv_with_part(lisa *l, char *msg, unsigned int msglength, int part);
#endif

/** \brief
 * The UDP version to receive data (port to receive data on is specified here).
 */
#ifdef DEBUG
#define lisa_recv_from(l, msg, msglength, port) LISA_RECV_FROM(l, msg, msglength, port, BREADCRUMB)
int  LISA_RECV_FROM(lisa *l, char *msg, unsigned int msglength, unsigned short port, char * file, char * line);
#else
int  lisa_recv_from(lisa *l, char *msg, unsigned int msglength, unsigned short port);
#endif

/** \brief
 * used to close a tube
 */
#ifdef DEBUG
#define lisa_close(l) LISA_CLOSE(l, BREADCRUMB)
void LISA_CLOSE(lisa *l, char * file, char * line);
#else
void lisa_close(lisa *l);
#endif

/** \brief
 * used to close the client connection for a server
 */
#ifdef DEBUG
#define lisa_close_client(l) LISA_CLOSE_CLIENT(l, BREADCRUMB)
void LISA_CLOSE_CLIENT(lisa *t, char * file, char * line);
#else
void lisa_close_client(lisa *t);
#endif

/** \brief
 * used to shutdown a tube. A shutdown indicates that you don't intend to
 * send any more data, but you can still receive any straggling incoming data.
 * This is unlike close which means you aren't going to receive data either.
 */
#ifdef DEBUG
#define lisa_shutdown(l) LISA_SHUTDOWN(l, BREADCRUMB)
void LISA_SHUTDOWN (lisa *l, char * file, char * line);
#else
void lisa_shutdown (lisa *l);
#endif

/** \brief
 * used to make a tube nonblocking so check to see if you got or sent anything
 * when you use this.
 */
#ifdef DEBUG
#define lisa_set_nonblocking(l) LISA_SET_NONBLOCKING(l, BREADCRUMB)
void LISA_SET_NONBLOCKING(lisa *l, char * file, char * line);
#else
void lisa_set_nonblocking(lisa *l);
#endif

/** \brief
 * returns the local port being used (if you care)
 */
#ifdef DEBUG
#define lisa_get_local_port(l) LISA_GET_LOCAL_PORT(l, BREADCRUMB)
unsigned short LISA_GET_LOCAL_PORT(lisa *l, char * file, char * line);
#else
unsigned short lisa_get_local_port(lisa *l);
#endif

/** \brief
 * called to get the local host name (if you care)
 */
#ifdef DEBUG
#define lisa_get_local_host(l, s) LISA_GET_LOCAL_HOST(l, s, BREADCRUMB)
void LISA_GET_LOCAL_HOST(lisa *l, char *s, char * file, char * line);
#else
void lisa_get_local_host(lisa *l, char *s);
#endif

/** \brief
 * reads until socket is closed; useful in reading an HTTP response
 * this also closes the socket on our side
 */
#ifdef DEBUG
#define lisa_read_response(l, s, size) LISA_READ_RESPONSE(l, s, size, BREADCRUMB)
int LISA_READ_RESPONSE(lisa *l, char *s, int size, char * file, char * line);
#else
int lisa_read_response(lisa *l, char *s, int size);
#endif

void lisa_set_wait_time(lisa *l, int t);

void lisa_set_to_vsock(lisa *l);

/* These are used internally, but you can reference them too, if you like. */

/* Socket Type */
#define LISA_TCP 1
#define LISA_UDP 2

/* State */
#define LISA_ERROR         -1
#define LISA_UNINITIALIZED 1
#define LISA_CREATED       2
#define LISA_BOUND         3
#define LISA_LISTENING     4
#define LISA_CONNECTED     5
#define LISA_TIMEOUT       6
#define LISA_ACCEPTED      7
#define LISA_OK            8
#define LISA_GOT_DATA      9
#define LISA_SENT_DATA     10
#define LISA_CLOSED        11

#define LISA_UNKNOWN       0
#define LISA_CLIENT        1
#define LISA_SERVER        2

#endif


