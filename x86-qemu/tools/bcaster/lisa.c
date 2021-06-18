/**
 * \file
 * The lisa implemenation. See lisa.h for more information.
 * David Hamilton, 2007
 */

#define ERRORS 1
#define TIMEOUTS 1

#include "lisa.h"
#include <strings.h>
#include <memory.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <pthread.h>


#ifdef WIN32
#else
#include <errno.h>
#include <string.h>
#endif

static int lisa_initialized = 0;

#ifdef DEBUG
#define COPY_MSG(fun,x) sprintf(l->debugmsg, "%s (Result: %s) (called from %s:%s)", fun, x, file, line)
#else
#define COPY_MSG(fun,x) sprintf(l->debugmsg, "%s (%s)", fun, x)
#endif

/**
 * Capture the last error (windows or Unix).
 */
#ifdef DEBUG
void lisa_set_error(lisa * l, char * fun, char * file, char * line) {
#else
void lisa_set_error(lisa * l, char * fun) {
#endif
#ifdef WIN32
  int rc;
  rc = WSAGetLastError();
  switch(rc) {
    case WSANOTINITIALISED: COPY_MSG(fun, "A successful WSAStartup call must occur before using this function"); break;
    case WSAENETDOWN: COPY_MSG(fun, "The network subsystem has failed."); break;
    case WSAEFAULT: COPY_MSG(fun, "The buffer pointed to by the buf or from parameters are not in the user address space, or the fromlen parameter is too small to accommodate the source address of the peer address."); break;
    case WSAEINTR: COPY_MSG(fun, "The (blocking) call was canceled through WSACancelBlockingCall."); break;
    case WSAEINPROGRESS: COPY_MSG(fun, "A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function."); break;
    case WSAEINVAL: COPY_MSG(fun, "The socket has not been bound with bind, or an unknown flag was specified, or MSG_OOB was specified for a socket with SO_OOBINLINE enabled, or (for byte stream-style sockets only) len was zero or negative."); break;
    case WSAEISCONN: COPY_MSG(fun, "The socket is connected. This function is not permitted with a connected socket, whether the socket is connection oriented or connectionless."); break;
    case WSAENETRESET: COPY_MSG(fun, "For a datagram socket, this error indicates that the time to live has expired."); break;
    case WSAENOTSOCK: COPY_MSG(fun, "The descriptor in the s parameter is not a socket."); break;
    case WSAEOPNOTSUPP: COPY_MSG(fun, "MSG_OOB was specified, but the socket is not stream-style such as type SOCK_STREAM, OOB data is not supported in the communication domain associated with this socket, or the socket is unidirectional and supports only send operations."); break;
    case WSAESHUTDOWN: COPY_MSG(fun, "The socket has been shut down; it is not possible to recvfrom on a socket after shutdown has been invoked with how set to SD_RECEIVE or SD_BOTH."); break;
    case WSAEWOULDBLOCK: COPY_MSG(fun, "The socket is marked as nonblocking and the recvfrom operation would block."); break;
    case WSAEMSGSIZE: COPY_MSG(fun, "The message was too large to fit into the buffer pointed to by the buf parameter and was truncated."); break;
    case WSAETIMEDOUT: COPY_MSG(fun, "The connection has been dropped, because of a network failure or because the system on the other end went down without notice."); break;
    case WSAECONNRESET: COPY_MSG(fun, "The virtual circuit was reset by the remote side executing a hard or abortive close. The application should close the socket; it is no longer usable. On a UDP-datagram socket this error indicates a previous send operation resulted in an ICMP Port Unreachable message."); break;
    case WSAEAFNOSUPPORT: COPY_MSG(fun, "The specified address family is not supported. For example, an application tried to create a socket for the AF_IRDA address family but an infrared adapter and device driver is not installed on the local computer."); break;
    case WSAEMFILE: COPY_MSG(fun, "No more socket descriptors are available."); break;
    case WSAENOBUFS: COPY_MSG(fun, "No buffer space is available. The socket cannot be created."); break;
    case WSAEPROTONOSUPPORT: COPY_MSG(fun, "The specified protocol is not supported."); break;
    case WSAEPROTOTYPE: COPY_MSG(fun, "The specified protocol is the wrong type for this socket."); break;
    case WSAESOCKTNOSUPPORT: COPY_MSG(fun, "The specified socket type is not supported in this address family."); break;
    case WSAEADDRINUSE: COPY_MSG(fun, "The socket's local address is already in use and the socket was not marked to allow address reuse with SO_REUSEADDR. This error usually occurs when executing bind, but could be delayed until this function if the bind was to a partially wildcard address (involving ADDR_ANY) and if a specific address needs to be committed at the time of this function."); break;
    case WSAEALREADY: COPY_MSG(fun, "A nonblocking connect call is in progress on the specified socket.");
    case WSAEADDRNOTAVAIL: COPY_MSG(fun, "The remote address is not a valid address (such as ADDR_ANY)."); break;
    case WSAECONNREFUSED: COPY_MSG(fun, "The attempt to connect was forcefully rejected."); break;
    case WSAENETUNREACH: COPY_MSG(fun, "The network cannot be reached from this host at this time."); break;
    case WSAEHOSTUNREACH: COPY_MSG(fun, "A socket operation was attempted to an unreachable host."); break;
    case WSAEACCES: COPY_MSG(fun, "An attempt to connect a datagram socket to broadcast address failed because setsockopt option SO_BROADCAST is not enabled."); break;
  }
#else
  COPY_MSG(fun, strerror(errno));
#endif
  return;
}

#ifdef DEBUG
#define SET_ERROR(fun) lisa_set_error(l,fun,file,line)
#else
#define SET_ERROR(fun) lisa_set_error(l,fun)
#endif

#ifdef WIN32
#define CHECK_FD_ERROR(fun,rc) if (l->fd == INVALID_SOCKET) { SET_ERROR(fun); l->state = LISA_ERROR; return rc;}
#define CHECK_RC_ERROR(fun,retval) if (rc == (int) INVALID_SOCKET) { SET_ERROR(fun); l->state = LISA_ERROR; return retval;}
#define CHECK_FD_ERROR_UNLOCK(fun,rc) if (l->fd == INVALID_SOCKET) { SET_ERROR(fun); l->state = LISA_ERROR; pthread_mutex_unlock(&(l->lock)); return rc;}
#define CHECK_RC_ERROR_UNLOCK(fun,retval) if (rc == (int) INVALID_SOCKET) { SET_ERROR(fun); l->state = LISA_ERROR; pthread_mutex_unlock(&(l->lock)); return retval;}
#else
#define CHECK_FD_ERROR(fun,rc) if (l->fd < 0) { SET_ERROR(fun); l->state = LISA_ERROR; return rc;}
#define CHECK_RC_ERROR(fun,retval) if (rc < 0) { SET_ERROR(fun); l->state = LISA_ERROR; return retval;}
#define CHECK_FD_ERROR_UNLOCK(fun,rc) if (l->fd < 0) { SET_ERROR(fun); l->state = LISA_ERROR; pthread_mutex_unlock(&(l->lock)); return rc;}
#define CHECK_RC_ERROR_UNLOCK(fun,retval) if (rc < 0) { SET_ERROR(fun); l->state = LISA_ERROR; pthread_mutex_unlock(&(l->lock)); return retval;}
#endif

#define CHECK_ERROR(x) if (!l || l->state < 0) return x;
#define CHECK_ERROR_UNLOCK(x) if (!l || l->state < 0) { pthread_mutex_unlock(&(l->lock)); return x; }

static int isnumber (char *c, int max) {
    int count = 0;
    while (*c && count < max) {
        if (*c<'0' || *c>'9') return 0;
        c++;
        count++;
    }
    return 1;
}

/**
 * start
 *
 */
#ifdef DEBUG
void LISA_START(char * file, char * line) {
#else
void lisa_start() {
#endif
  if (lisa_initialized) return;
  lisa_initialized = 1;
#ifdef WIN32
  WORD winsock_version;
  WSADATA winsock_data;
  int rc;
  winsock_version = MAKEWORD(2, 2);
  rc = WSAStartup(winsock_version, &winsock_data);
  if (rc != 0) {
    WSACleanup();
  }
#endif
}

/**
 * stop (winsock)
 *
 */
#ifdef DEBUG
void LISA_STOP(char * file, char * line) {
#else
void lisa_stop() {
#endif
  lisa_initialized = 0; /* although you really can't start winsock again; windows will complain if you do */
#ifdef WIN32
  WSACleanup();
#endif
}

/**
 * initialize a lisa
 *
 */
#ifdef DEBUG
void LISA_OPEN(lisa* l, char * file, char * line) {
#else
void lisa_open(lisa* l) {
#endif
  if (!l) return;
  l->state = LISA_UNINITIALIZED;
  l->client_state = LISA_UNINITIALIZED;
  l->wait_time.tv_sec = 30;
  l->wait_time.tv_usec = 0;
  l->num_clients=0;
  l->clsvr = LISA_UNKNOWN;
  l->vsock = 0;
  COPY_MSG("lisa_open", "OK");
}

void lisa_set_to_vsock(lisa *l) {
    l->vsock = 1;
}

/**
 * Setup the lisa for TCP, create local and remote addresses, initialize the flags to nothing.
 */
#ifdef DEBUG
void LISA_SETUP_TCP(lisa *l, char * file, char * line) {
#else
void lisa_setup_tcp(lisa *l) {
#endif
  CHECK_ERROR();
  if (!lisa_initialized) lisa_start();
  //FD_ZERO(&(l->fdset));
  if (l->vsock) {
      l->fd  = socket(AF_VSOCK, SOCK_STREAM, 0);
      CHECK_FD_ERROR("lisa_setup_tcp vsock", )
      l->state = LISA_CREATED;
      memset(&(l->local_vaddr),  0, sizeof(l->local_vaddr));
      memset(&(l->remote_vaddr), 0, sizeof(l->remote_vaddr));
      l->local_vaddr.svm_family  = AF_VSOCK;
      l->remote_vaddr.svm_family = AF_VSOCK;
      l->type = LISA_TCP;
      l->flags = 0;
      l->state = LISA_CREATED;
      l->client_state = LISA_UNINITIALIZED;
      lisa_set_nonblocking(l);
  }
  else {
      l->fd  = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
      CHECK_FD_ERROR("lisa_setup_tcp", )
      l->state = LISA_CREATED;
      memset(&(l->local_addr),  0, sizeof(l->local_addr));
      memset(&(l->remote_addr), 0, sizeof(l->remote_addr));
      l->local_addr.sin_family  = AF_INET;
      l->remote_addr.sin_family = AF_INET;
      l->type = LISA_TCP;
      l->flags = 0;
      l->state = LISA_CREATED;
      l->client_state = LISA_UNINITIALIZED;
      lisa_set_nonblocking(l);
  }
}


/**
 * Setup the lisa for UDP, create local and remote addresses, initialize flags to nothing.
 */
#ifdef DEBUG
void LISA_SETUP_UDP(lisa *l, char * file, char * line) {
#else
void lisa_setup_udp(lisa *l) {
#endif
  CHECK_ERROR();
  //FD_ZERO(&(l->fdset));
  if (!lisa_initialized) lisa_start();
  if (l->vsock) {
      l->fd  = socket(AF_VSOCK, SOCK_DGRAM, 0);
      CHECK_FD_ERROR("lisa_setup_udp vsock",)
      memset(&(l->local_vaddr),  0, sizeof(l->local_vaddr));
      memset(&(l->remote_vaddr), 0, sizeof(l->remote_vaddr));
      l->local_vaddr.svm_family  = AF_VSOCK;
      l->remote_vaddr.svm_family = AF_VSOCK;
  }
  else {
      l->fd  = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
      CHECK_FD_ERROR("lisa_setup_udp",)
      memset(&(l->local_addr),  0, sizeof(l->local_addr));
      memset(&(l->remote_addr), 0, sizeof(l->remote_addr));
      l->local_addr.sin_family  = AF_INET;
      l->remote_addr.sin_family = AF_INET;
  }
  l->type = LISA_UDP;
  l->flags = 0;
  l->state = LISA_CREATED;
  COPY_MSG("lisa_setup_udp", "OK");
}

/**
 * Set the remote address to the destination addr and port, connect,
 * and then get the local address of the connection.
 * Note that if the lisa has not yet been created, it will be
 * automatically set to TCP.
 */
#ifdef DEBUG
void LISA_CONNECT(lisa *l, char * addr, unsigned short port, char * file, char * line) {
#else
void lisa_connect(lisa *l, char * addr, unsigned short port) {
#endif
  int rc;
  struct timeval timeout;
  struct timeval * tp;
  SOCKET         this_fd;
  fd_set         this_fdset;
#ifdef WIN32
  int size;
#else
  unsigned int size;
#endif
  /* LLLLLLLLLLLLLLLLLLLLLLLL
   * PRE-SELECT
   * LLLLLLLLLLLLLLLLLLLLLLLL
   */
  pthread_mutex_lock(&(l->lock));
  CHECK_ERROR_UNLOCK();
  this_fd = l->fd;
  l->clsvr = LISA_CLIENT;
  if (l->state < LISA_CREATED) {
     lisa_setup_tcp(l);
     CHECK_ERROR_UNLOCK();
  }
#ifdef WIN32
  if (addr) l->remote_addr.sin_addr.s_addr = inet_addr(addr);
#else
  if (l->vsock) {
    /* convert addr from text number to binary */
    if (!isnumber(addr, 10)) {SET_ERROR("lisa connect failed"); l->state = LISA_ERROR; return;}
    l->remote_vaddr.svm_cid = atol(addr);
    l->remote_vaddr.svm_port = port;  /* vsocks are always in host byte order */
    rc = connect(l->fd, (struct sockaddr *) &(l->remote_vaddr), sizeof(l->remote_vaddr));
    //printf("connection rc: %d\n", rc);
    //printf("error connecting: %s\n", strerror(errno));
}
  else {
    /* convert dotted ip address to binary */
    if (addr) if(inet_pton(AF_INET, addr, &(l->remote_addr.sin_addr))<=0) {SET_ERROR("lisa connect failed"); l->state = LISA_ERROR; return;}
    l->remote_addr.sin_port = htons(port);
    rc = connect(l->fd, (struct sockaddr *) &(l->remote_addr), sizeof(l->remote_addr));
  }
#endif
#ifdef WIN32
  if (rc <0 && WSAGetLastError() != WSAEWOULDBLOCK) {
#else
  if (rc <0 && errno != EINPROGRESS) {
#endif
	  SET_ERROR("lisa_connect failed");
	  l->state = LISA_ERROR;
	  pthread_mutex_unlock(&(l->lock));
	  return;
  }
  if (l->wait_time.tv_sec == 0) tp = NULL;
  else {
    timeout.tv_sec = l->wait_time.tv_sec;
    timeout.tv_usec = l->wait_time.tv_usec;
	tp = &(timeout);
  }
  pthread_mutex_unlock(&(l->lock));
  /* LLLLLLLLLLLLLLLLLLLLLLLL
   * SELECT
   * LLLLLLLLLLLLLLLLLLLLLLLL
   */
  FD_ZERO(&this_fdset);
  FD_SET(this_fd, &this_fdset);
  rc = select(this_fd+1, NULL, &this_fdset, NULL, tp);
  if (select(this_fd+1, NULL, &this_fdset, NULL, tp) <= 0) {
    COPY_MSG("lisa_connect", "timeout waiting for connect");
    pthread_mutex_lock(&(l->lock));
	l->state = LISA_ERROR;
    pthread_mutex_unlock(&(l->lock));
    return;
  }
  /* LLLLLLLLLLLLLLLLLLLLLLLL
   * POST-SELECT
   * LLLLLLLLLLLLLLLLLLLLLLLL
   */
  pthread_mutex_lock(&(l->lock));
  if (!FD_ISSET(this_fd, &this_fdset)) COPY_MSG("lisa_connect", "fd not set");
  if (l->vsock) {
    size = sizeof(l->local_vaddr);
    rc = getsockname(this_fd, (struct sockaddr *) &(l->local_vaddr), &size);
  }
  else {
    size = sizeof(l->local_addr);
    rc = getsockname(this_fd, (struct sockaddr *) &(l->local_addr), &size);
  }
  if (rc<0) {
    printf("error getsockname: %s\n", strerror(errno));
  }
  l->state = LISA_CONNECTED;
  COPY_MSG("lisa_connect", "OK");
  pthread_mutex_unlock(&(l->lock));
}

/** \brief
 * called in a client to connect to a host using a name (not an ip address)
 */
#ifdef DEBUG
void LISA_CONNECT_BY_NAME(lisa *l, char * host_name, unsigned short port, char * file, char * line) {
#else
void lisa_connect_by_name(lisa *l, char * host_name, unsigned short port) {
#endif
	struct hostent *host;
  	CHECK_ERROR();
  	l->clsvr = LISA_CLIENT;
  	if (l->state < LISA_CREATED) {
    	lisa_setup_tcp(l);
    	CHECK_ERROR();
  	}
  	if (l->vsock) {
        lisa_connect(l,host_name,port);
  	}
  	else {
        if ((host = gethostbyname(host_name)) != NULL) {
            l->remote_addr.sin_addr.s_addr = *((unsigned long *) host->h_addr_list[0]);
            lisa_connect(l,NULL,port);
        }
        else {
          l->state = LISA_ERROR;
          COPY_MSG("lisa_connect_by_name", "error getting host name");
        }
  	}
  COPY_MSG("lisa_connect_by_name", "OK");
}



/**
 * In addition to calling the regular listen function, this first
 * binds the socket to a local address if not already bound.
 * Note that this works for UDP or TCP sockets; for UDP it just
 * binds the socket, making it ready to receive datagrams to.
 * Note that this will call the appropriate setup function if
 * the lisa is not yet created and that if the type has not yet
 * been set, it will default to TCP if UDP is not set.
 *
 */
#ifdef DEBUG
void LISA_LISTEN(lisa *l, unsigned short port, char * file, char * line) {
#else
void lisa_listen(lisa *l, unsigned short port) {
#endif
  int rc;
  CHECK_ERROR();
  l->clsvr = LISA_SERVER;
  if (l->state < LISA_CREATED) {
    if (l->type == LISA_UDP) lisa_setup_udp(l);
    else lisa_setup_tcp(l);
  }
  CHECK_ERROR();
  if (l->state < LISA_BOUND) {
    if (l->vsock) {
        l->local_vaddr.svm_cid = VMADDR_CID_ANY;
        l->local_vaddr.svm_port = port;  /* vsock always in host byte order */
        //printf("port: %d\n", l->local_vaddr.svm_port);
        rc = bind(l->fd, (struct sockaddr *) &(l->local_vaddr), sizeof(l->local_vaddr));
        //printf("bind rc: %d\n", rc);
        CHECK_RC_ERROR("lisa_listen", );
    }
    else {
        l->local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        l->local_addr.sin_port = htons(port);
        rc = bind(l->fd, (struct sockaddr *) &(l->local_addr), sizeof(l->local_addr));
        CHECK_RC_ERROR("lisa_listen", );
    }
    l->state = LISA_BOUND;
  }
  rc = listen(l->fd, 1);
  //printf("listen rc: %d\n", rc);
  CHECK_RC_ERROR("lisa_listen", )
  COPY_MSG("lisa_listen", "OK");
}

/**
 * In addition to calling the regular accept function, this also calls
 * listen automatically if needed.
 */
#ifdef DEBUG
int LISA_ACCEPT(lisa *l, unsigned short port, char * file, char * line) {
#else
int lisa_accept(lisa *l, unsigned short port) {
#endif
  SOCKET  this_fd;
  fd_set  this_fdset;
  int rc;
#ifdef WIN32
  int size;
#else
  unsigned int size;
#endif
  struct timeval timeout;
  struct timeval * tp;
  /* LLLLLLLLLLLLLLLLLLLLLLLL
   * PRE-SELECT
   * LLLLLLLLLLLLLLLLLLLLLLLL
   */
  pthread_mutex_lock(&(l->lock));
  this_fd = l->fd;
  CHECK_ERROR_UNLOCK(0);
  if (l->state < LISA_LISTENING) {
    lisa_listen(l, port);
    CHECK_ERROR_UNLOCK(0);
	l->state = LISA_LISTENING;
  }
  /*
  if (l->vsock) {
      size = sizeof(l->remote_vaddr);
      rc = accept(l->fd, (struct sockaddr *) &(l->remote_vaddr), &size);
      if (rc <= 0) {
        COPY_MSG("lisa_accept", strerror(errno));
      }
      else {
        l->client_fd = rc;
        l->state = LISA_ACCEPTED;
        l->client_state = LISA_CONNECTED;
        l->client_fds[l->num_clients] = l->client_fd;
        l->num_clients++;
      }
  }
  else {
  */
      if (l->wait_time.tv_sec == 0) tp = NULL;
      else {
        timeout.tv_sec = l->wait_time.tv_sec;
        timeout.tv_usec = l->wait_time.tv_usec;
        tp = &(timeout);
      }
      pthread_mutex_unlock(&(l->lock));
  /* LLLLLLLLLLLLLLLLLLLLLLLL
   * SELECT
   * LLLLLLLLLLLLLLLLLLLLLLLL
   */
      FD_ZERO(&this_fdset);
      FD_SET(this_fd, &this_fdset);
      rc = select(this_fd+1, &this_fdset, NULL, NULL, tp);

  /* LLLLLLLLLLLLLLLLLLLLLLLL
   * POST-SELECT
   * LLLLLLLLLLLLLLLLLLLLLLLL
   */
      pthread_mutex_lock(&(l->lock));
      CHECK_RC_ERROR_UNLOCK("lisa_accept", 0);
      switch (rc) {
        case 0: l->state = LISA_TIMEOUT;
                COPY_MSG("lisa_accept", "Timeout");
                rc = 1;
                break;
        case -1: l->state = LISA_ERROR;
                 COPY_MSG("lisa_accept", "Error accepting");
                 rc = 0;
                 break;
        default: l->state = LISA_ACCEPTED;
                 if (!FD_ISSET(this_fd, &this_fdset)) {
                   COPY_MSG("lisa_accept", "accepted but fd not set");
                   l->client_state = LISA_ERROR;
                   pthread_mutex_unlock(&(l->lock));
                   return 0;
                 }
                 if (l->vsock) {
                     size = sizeof(l->remote_vaddr);
                     rc = accept(l->fd, (struct sockaddr *) &(l->remote_vaddr), &size);
                     if (rc <= 0) {
                       COPY_MSG("lisa_accept", strerror(errno));
                     }
                     else {
                       l->client_fd = rc;
                       //printf("added client: %d\n", l->client_fd);
                       l->state = LISA_ACCEPTED;
                       l->client_state = LISA_CONNECTED;
                       l->client_fds[l->num_clients] = l->client_fd;
                       l->num_clients++;
                       //printf("added client: %d (%d)\n", l->client_fd, l->num_clients);
                     }
                 }
                 else {
                     size = sizeof(l->remote_addr);
                     l->client_fd = accept(l->fd, (struct sockaddr *) &(l->remote_addr), &size);
                     if (l->client_fd <= 0) COPY_MSG("lisa_accept", "Error accepting connection");
                     l->client_state = LISA_CONNECTED;
                     l->client_fds[l->num_clients] = l->client_fd;
                     l->num_clients++;
                     rc = 1;
                 }
      }
  /*
  }
  */

  COPY_MSG("lisa_accept", "OK");
  pthread_mutex_unlock(&(l->lock));
  return rc;
}

/**
 * Basically just calls the regular send function. It does ensure that
 * the socket has been connected first.
 * Note that in the case of udp, the socket is conceptually connected
 * when the remote address was set by a previous recv_from.
 */
#ifdef DEBUG
int LISA_SEND(lisa *l, char *msg, unsigned int msglength, char * file, char * line) {
#else
int lisa_send(lisa *l, char *msg, unsigned int msglength) {
#endif
  int rc;
  unsigned int bytes_sent;
  struct timeval timeout;
  struct timeval * tp;
  SOCKET this_fd;
  SOCKET this_socket;
  fd_set this_fdset;
  char   this_type;
  int    this_flags;
  int    this_state;
  /* LLLLLLLLLLLLLLLLLLLLLLLL
   * PRE-SELECT
   * LLLLLLLLLLLLLLLLLLLLLLLL
   */
  pthread_mutex_lock(&(l->lock));
  this_fd = l->fd;
  this_flags = l->flags;
  if (l->state != LISA_ACCEPTED) this_state = l->state;
  else this_state = l->client_state;
  CHECK_ERROR_UNLOCK(-1);
  if (l->state < LISA_CONNECTED) {
        pthread_mutex_unlock(&(l->lock));
        return LISA_ERROR;
  }
  if (l->clsvr == LISA_CLIENT) this_socket = this_fd;
  else this_socket = l->client_fd;  //note that this defaults to the most recently set fd from recv (but could be overwritten as in cast)
  this_type = l->type;
  if (l->wait_time.tv_sec == 0) tp = NULL;
  else {
     timeout.tv_sec = l->wait_time.tv_sec;
     timeout.tv_usec = l->wait_time.tv_usec;
     tp = &(timeout);
  }
  pthread_mutex_unlock(&(l->lock));
  /* LLLLLLLLLLLLLLLLLLLLLLLL
   * SELECT
   * LLLLLLLLLLLLLLLLLLLLLLLL
   */
  if (this_type == LISA_TCP) {
	bytes_sent = 0;
    FD_ZERO(&this_fdset);
    FD_SET(this_socket, &this_fdset);
	  while (bytes_sent < msglength && this_state != LISA_ERROR) {
	    rc = select(this_socket+1, NULL, &this_fdset, NULL, tp);
	    if (rc <= 0) {
            pthread_mutex_lock(&(l->lock));
            COPY_MSG("lisa_send", "timeout sending");
		    this_state = LISA_TIMEOUT;
            pthread_mutex_unlock(&(l->lock));
	    }
	    else {
          if (!FD_ISSET(this_socket, &this_fdset)) {
            pthread_mutex_lock(&(l->lock));
	        COPY_MSG("lisa_send", "unexpected fd not set");
		    this_state = LISA_ERROR;
            pthread_mutex_unlock(&(l->lock));
  	      }
		  else { /* all ok */
            bytes_sent += send(this_socket, &(msg[bytes_sent]), msglength-bytes_sent, this_flags);
            this_state = LISA_SENT_DATA;
          }
	    }
    } //while
    rc = bytes_sent;
    //printf("rc: %d\n", rc);
  }
  else {  //not TCP
    pthread_mutex_lock(&(l->lock));
    if (l->vsock) {
        rc = sendto(this_socket, msg, msglength, this_flags,
                      (struct sockaddr *) &(l->remote_vaddr), sizeof(l->remote_vaddr));
    }
    else {
        rc = sendto(this_socket, msg, msglength, this_flags,
                      (struct sockaddr *) &(l->remote_addr), sizeof(l->remote_addr));
    }
    this_state = LISA_SENT_DATA;
    pthread_mutex_unlock(&(l->lock));
  }
  if (rc < 0) this_state = LISA_ERROR;
  /* LLLLLLLLLLLLLLLLLLLLLLLL
   * POST-SELECT
   * LLLLLLLLLLLLLLLLLLLLLLLL
   */
  pthread_mutex_lock(&(l->lock));
  if (l->state != LISA_ACCEPTED) l->state = this_state;
  else l->client_state = this_state;
  COPY_MSG("lisa_send", "OK");
  pthread_mutex_unlock(&(l->lock));
  return rc;
}

/**
 * Sends same msg to all clients (generally only useful for server; for client is identical to send)
 */
#ifdef DEBUG
int LISA_CAST(lisa *l, char *msg, unsigned int msglength, char * file, char * line) {
#else
int lisa_cast(lisa *l, char *msg, unsigned int msglength) {
#endif
  //printf("in cast\n");
  int last_client;
  pthread_mutex_lock(&(l->lock));
  if (l->clsvr == LISA_CLIENT) {
        pthread_mutex_unlock(&(l->lock));
        lisa_send(l,msg,msglength);
        return msglength;
  }
  else {
    last_client = l->client_fd;
    for (int i=0; i<l->num_clients; i++) {
     //fprintf(stderr, "%d ", i);
      if (l->client_fds[i] != last_client) {
        l->client_fd = l->client_fds[i];
        pthread_mutex_unlock(&(l->lock));
        lisa_send(l, msg, msglength);
        pthread_mutex_lock(&(l->lock));
        //printf("sent\n");
      }
    }
    pthread_mutex_unlock(&(l->lock));
    return msglength;
  }
}


/**
 * Basically the same as bind for UDP sockets, except that it automatically
 * sets the address to be any and handles errors.
 */
#ifdef DEBUG
void LISA_SEND_FROM(lisa *l, unsigned short port, char * file, char * line) {
#else
void lisa_send_from(lisa *l, unsigned short port) {
#endif
  int rc;
  CHECK_ERROR();
  if (l->state < LISA_CREATED) {
    l->state = LISA_ERROR;
	COPY_MSG("lisa_send_from", "not created");
    return;
  }
  if (l->vsock) {
      l->local_vaddr.svm_cid = VMADDR_CID_ANY;
      l->local_vaddr.svm_port = port;   /* vsock is always host order */
      rc = bind(l->fd, (struct sockaddr *) &(l->local_vaddr), sizeof(l->local_vaddr));
  }
  else {
      l->local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
      l->local_addr.sin_port = htons(port);
      rc = bind(l->fd, (struct sockaddr *) &(l->local_addr), sizeof(l->local_addr));
  }
  CHECK_RC_ERROR("lisa_send_from", )
  l->state = LISA_BOUND;
  COPY_MSG("lisa_send_from", "OK");
  return;
}

/**
 * Basically the same as send_to except that it sets up the address and handles errors.
 */
#ifdef DEBUG
int LISA_SEND_TO(lisa *l, char *msg, unsigned int msglength, char *addr, unsigned short port, char * file, char * line) {
#else
int lisa_send_to(lisa *l, char *msg, unsigned int msglength, char *addr, unsigned short port) {
#endif
  int rc;
  pthread_mutex_lock(&(l->lock));
  CHECK_ERROR_UNLOCK(-1);
  if (l->state < LISA_CREATED) {
    lisa_setup_udp(l);
	CHECK_ERROR_UNLOCK(-1);
  }
  if (l->vsock) {
      if (!isnumber(addr, 10)) {SET_ERROR("lisa connect failed"); l->state = LISA_ERROR; pthread_mutex_unlock(&(l->lock));return -1;}
      l->remote_vaddr.svm_cid = atol(addr);
      l->remote_vaddr.svm_port = port;  /* vsock is always host byte order */
      rc = sendto(l->fd, msg, msglength, l->flags,
                    (struct sockaddr *) &(l->remote_vaddr), sizeof(l->remote_vaddr));
  }
  else {
      l->remote_addr.sin_addr.s_addr = inet_addr(addr);
      l->remote_addr.sin_port = htons(port);
      rc = sendto(l->fd, msg, msglength, l->flags,
                    (struct sockaddr *) &(l->remote_addr), sizeof(l->remote_addr));
  }
  l->state = LISA_CONNECTED;
  CHECK_RC_ERROR_UNLOCK("lisa_send_to", rc)
  COPY_MSG("lisa_send_to", "OK");
  pthread_mutex_unlock(&(l->lock));
  return rc;
}

/**
 * Basically the same as recv or recv_from except that it handles errors.
 * Note that in the case of UDP, conceptually the socket is connected because
 * the remote address was set by a previous send_to;
 * and errors are captured in the state field, not the return code (which is never negative)
 */
#ifdef DEBUG
int LISA_RECV(lisa *l, char *msg, unsigned int msglength, char * file, char * line) {
#else
int lisa_recv(lisa *l, char *msg, unsigned int msglength) {
#endif
  return lisa_recv_with_part(l, msg, msglength, 12);
}

#ifdef DEBUG
int LISA_RECV_PART1(lisa *l, char * file, char * line) {
#else
int lisa_recv_part1(lisa *l) {
#endif
  return lisa_recv_with_part(l, NULL, 0, 1);
}

#ifdef DEBUG
int LISA_RECV_PART2(lisa *l, char *msg, unsigned int msglength, char * file, char * line) {
#else
int lisa_recv_part2(lisa *l, char *msg, unsigned int msglength) {
#endif
  return lisa_recv_with_part(l, msg, msglength, 2);
}

#ifdef DEBUG
int LISA_RECV_WITH_PART(lisa *l, char *msg, unsigned int msglength, int part, char * file, char * line) {
#else
int lisa_recv_with_part(lisa *l, char *msg, unsigned int msglength, int part) {
#endif
  int rc, this_state;
  struct timeval timeout;
  struct timeval * tp;
  SOCKET max_fd, this_fd;
  fd_set this_fdset;
  this_state = LISA_ERROR;
  rc = -1;
#ifdef WIN32
  int size;
#else
  unsigned int size;
#endif
  /* LLLLLLLLLLLLLLLLLLLLLLLL
   * PRE-SELECT
   * LLLLLLLLLLLLLLLLLLLLLLLL
   */
  if (part == 2) goto part2;
  pthread_mutex_lock(&(l->lock));
  CHECK_ERROR_UNLOCK(-1);
  this_fd = l->fd;
  this_state = l->state;
  if (l->state < LISA_CONNECTED || l->clsvr == LISA_UNKNOWN) {
    COPY_MSG("lisa_recv", "not connected");
    pthread_mutex_unlock(&(l->lock));
    return 0;
  }
  this_fd = 0;
  if (l->wait_time.tv_sec == 0) tp = NULL;
  else {
    timeout.tv_sec = l->wait_time.tv_sec;
    timeout.tv_usec = l->wait_time.tv_usec;
	tp = &(timeout);
  }
  FD_ZERO(&this_fdset);
  if (l->clsvr == LISA_CLIENT) {
    FD_SET(this_fd, &this_fdset);
    max_fd=this_fd+1;
  }
  else {
    max_fd=0;
    for (int i=0; i<l->num_clients; i++) {
        this_fd=l->client_fds[i];
        //printf("selecting on: %d\n", this_fd);
        FD_SET(this_fd, &this_fdset);
        if (max_fd<this_fd) max_fd=this_fd;
    }
    max_fd++;
  }
  pthread_mutex_unlock(&(l->lock));
  /* LLLLLLLLLLLLLLLLLLLLLLLL
   * SELECT
   * LLLLLLLLLLLLLLLLLLLLLLLL
   */
  rc = select(max_fd, &this_fdset, NULL, NULL, tp);
  /* LLLLLLLLLLLLLLLLLLLLLLLL
   * POST-SELECT
   * LLLLLLLLLLLLLLLLLLLLLLLL
   */
  pthread_mutex_lock(&(l->lock));
  if (rc <= 0) {
    COPY_MSG("lisa_recv", "timeout receiving");
    this_state = LISA_TIMEOUT;
	rc = -1;
  }
  else {
    /* this_fd := fd for sending data, depending on if we are a client or a server */
    if (l->clsvr == LISA_CLIENT) {
        this_fd=l->fd;
    }
    else { //we must be a server
        l->client_fd=0;
        for (int i=0; i<l->num_clients; i++) {
            if (FD_ISSET(l->client_fds[i], &this_fdset)) {
                l->client_fd = l->client_fds[i];
                break;
            }
        }
        if (!l->client_fd) {
            COPY_MSG("lisa_recv", "Unexpected FD not set receiving");
        }
        else {
            this_fd=l->client_fd;
        }
    }
    l->this_fd = this_fd;
    if (part ==1) return  this_fd;
    part2:
    this_fd = l->this_fd;
    /* now that we have the socket for receiving data, do the receive and set the current state, depending on if tcp or udp */
    if (this_fd) {
        if (l->type == LISA_TCP)  {
            rc = recv(this_fd, msg, msglength, 0);
            //printf("rc: %d, this_fd: %d\n", rc, this_fd);
            if (rc <= 0) {
                if (rc < 0) {
                    printf("%s\n", strerror(errno));
                    COPY_MSG("lisa_recv", "receive failed");
                    this_state = LISA_ERROR;
                }
                else this_state = LISA_CLOSED;  /* if select passes but no data then socket is at EOF */
            }
            else {
                this_state = LISA_GOT_DATA;
                //todo: handle tv flag (and add flags param)
            }

        }
        else { //UDP
            size = sizeof(l->remote_addr);
            rc = recvfrom(this_fd, msg, msglength, l->flags, (struct sockaddr*) &l->remote_addr, &size);
            this_state = LISA_GOT_DATA;
        }
    }
  } //not timeout receiving
  if (l->clsvr == LISA_CLIENT) l->state = this_state;
  else l->client_state = this_state;
  COPY_MSG("lisa_recv", "OK");
  pthread_mutex_unlock(&(l->lock));
  return rc;
}

/**
 * Basically the same as recv_from except that it automatically binds
 * if needed, sets up the local address and handles errors. The remote
 * address is retrieved from the send_to call.
 */
#ifdef DEBUG
int LISA_RECV_FROM(lisa *l, char *msg, unsigned int msglength, unsigned short port, char * file, char * line) {
#else
int lisa_recv_from(lisa *l, char *msg, unsigned int msglength, unsigned short port) {
#endif
#ifdef WIN32
  int size;
#else
  unsigned int size;
#endif
  struct timeval timeout;
  struct timeval * tp;
  int this_fd;
  int rc;
  fd_set this_fdset;
  /* LLLLLLLLLLLLLLLLLLLLLLLL
   * PRE-SELECT
   * LLLLLLLLLLLLLLLLLLLLLLLL
   */
  pthread_mutex_lock(&(l->lock));
  this_fd = l->fd;
  CHECK_ERROR_UNLOCK(-1);
  if (l->state < LISA_CREATED) {   pthread_mutex_unlock(&(l->lock)); return -1; }
  if (l->type != LISA_UDP) {   pthread_mutex_unlock(&(l->lock)); return -2; }
  if (l->state < LISA_BOUND) {
    if (l->vsock) {
        l->local_vaddr.svm_cid = VMADDR_CID_ANY;
        l->local_vaddr.svm_port = port;  /* vsock is always host byte order */
        rc = bind(this_fd, (struct sockaddr *) &(l->local_vaddr), sizeof(l->local_vaddr));
    }
    else {
        l->local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        l->local_addr.sin_port = htons(port);
        rc = bind(this_fd, (struct sockaddr *) &(l->local_addr), sizeof(l->local_addr));
    }
    CHECK_RC_ERROR_UNLOCK("lisa_recv_from", rc)
    l->state = LISA_BOUND;
  }
  FD_SET(this_fd, &this_fdset);
  if (l->wait_time.tv_sec == 0) tp = NULL;
  else {
    timeout.tv_sec = l->wait_time.tv_sec;
    timeout.tv_usec = l->wait_time.tv_usec;
	tp = &(timeout);
  }
  pthread_mutex_unlock(&(l->lock));
  /* LLLLLLLLLLLLLLLLLLLLLLLL
   * SELECT
   * LLLLLLLLLLLLLLLLLLLLLLLL
   */
  rc = select(this_fd+1, &this_fdset, NULL, NULL, tp);
  /* LLLLLLLLLLLLLLLLLLLLLLLL
   * POST-SELECT
   * LLLLLLLLLLLLLLLLLLLLLLLL
   */
  pthread_mutex_lock(&(l->lock));
  if (rc <= 0) {
    COPY_MSG("lisa_recv_from", "timeout receiving");
    pthread_mutex_unlock(&(l->lock));
    return 0;
  }
  if (!FD_ISSET(this_fd, &this_fdset)) COPY_MSG("lisa_recv_from", "fd not set");
  if (l->vsock) {
    size = sizeof(l->local_vaddr);
    rc = recvfrom(this_fd, msg, msglength, l->flags,
                     (struct sockaddr *) &(l->remote_vaddr), &size);
  }
  else {
    size = sizeof(l->local_addr);
    rc = recvfrom(this_fd, msg, msglength, l->flags,
                     (struct sockaddr *) &(l->remote_addr), &size);
  }
  if (l->blocking) { CHECK_RC_ERROR_UNLOCK("lisa_recv_from", rc) }
  l->state = LISA_GOT_DATA;
  COPY_MSG("lisa_recv_from", "OK");
  pthread_mutex_unlock(&(l->lock));
  return rc;
}

/**
 * Lingering close
 */
#ifdef DEBUG
void LISA_CLOSE(lisa *l, char * file, char * line) {
#else
void lisa_close(lisa *l) {
#endif
#ifndef WIN32
  struct linger linger = {1,5};
#endif
  CHECK_ERROR();
  if (l->state < LISA_CONNECTED) return;
#ifdef WIN32
  /* setsockopt(l->fd, SOL_SOCKET, SO_LINGER, (char *) &linger, sizeof(linger)); */
  closesocket(l->fd);
#else
  setsockopt(l->fd, SOL_SOCKET, SO_LINGER, &linger, sizeof(linger));
  for (int i=0; i<l->num_clients; i++) {
    close(l->client_fds[i]);
  }
  close(l->fd);
#endif
  l->state = LISA_UNINITIALIZED;
  COPY_MSG("lisa_close", "OK");
}

/**
 * Lingering close for a client connection; server goes back to listening
 */
#ifdef DEBUG
void LISA_CLOSE_CLIENT(lisa *l, char * file, char * line) {
#else
void lisa_close_client(lisa *l) {
#endif
  int i, j, found;
  struct linger linger = {1,5};
  CHECK_ERROR();
  if (l->client_state < LISA_CONNECTED) return;
#ifdef WIN32
  setsockopt(l->client_fd, SOL_SOCKET, SO_LINGER, (char *) &linger, sizeof(linger));
  closesocket(l->client_fd);
#else
  setsockopt(l->client_fd, SOL_SOCKET, SO_LINGER, &linger, sizeof(linger));
  close(l->client_fd);
#endif
  found = 0;
  for (i=0; i<l->num_clients && !found; i++) found = (l->client_fds[i] == l->client_fd);
  for (j=i+1; j<l->num_clients; j++) l->client_fds[j-1] = l->client_fds[j];
  l->num_clients--;
  l->client_state = LISA_UNINITIALIZED;
  l->state = LISA_LISTENING;
  COPY_MSG("lisa_close_client", "OK");
}


/**
 * Just does what it says.
 */
#ifdef DEBUG
void LISA_SET_NONBLOCKING(lisa *l, char * file, char * line) {
#else
void lisa_set_nonblocking(lisa *l) {
#endif
  int rc;
#ifdef WIN32
  unsigned long arg;
  CHECK_ERROR();
  arg = 1;
  rc = ioctlsocket(l->fd, FIONBIO, &arg);
  CHECK_RC_ERROR("lisa_set_nonblocking", )
  if (rc == SOCKET_ERROR) {
    COPY_MSG("lisa_set_nonblocking", "error");
    l->state = LISA_ERROR;
    return;
  }
  l->blocking = 0;
  COPY_MSG("lisa_set_nonblocking", "OK");
}
#else
  /*
  l->flags = fcntl(l->fd, F_GETFL, 0);
  if (l->flags < 0) {
    COPY_MSG("lisa_set_nonblocking", "error in flags");
    l->state = LISA_ERROR;
    return;
  }
  l->flags |= O_NONBLOCK;
  rc = fcntl(l->fd, F_SETFL, l->flags);
  */
  rc = fcntl(l->fd, F_SETFL, O_NONBLOCK);
  if (rc < 0) {
    printf("error setting flags\n");
    COPY_MSG("lisa_set_nonblocking", "error in flags after setting");
    l->state = LISA_ERROR;
    return;
  }
  l->blocking = 0;
  COPY_MSG("lisa_set_nonblocking", "OK");
}
#endif


/**
 * Just return the local port.
 */
#ifdef DEBUG
unsigned short LISA_GET_LOCAL_PORT(lisa *l, char * file, char * line) {
#else
unsigned short lisa_get_local_port(lisa *l) {
#endif
  CHECK_ERROR(0);
  if (l->vsock) return l->local_vaddr.svm_port;
  else return l->local_addr.sin_port;
}


/**
 * Just return the local address.
 */
#ifdef DEBUG
void LISA_GET_LOCAL_HOST(lisa *l, char *s, char * file, char * line) {
#else
void lisa_get_local_host(lisa *l, char *s) {
#endif
  char *ip_stack_addr_string;
  char vmaddr[10];
  CHECK_ERROR();
  if (l->vsock) {
    snprintf(vmaddr, 10, "%d", l->local_vaddr.svm_cid);
    strcpy(s, vmaddr);
  }
  else {
    ip_stack_addr_string = inet_ntoa(l->local_addr.sin_addr);
    strcpy(s, ip_stack_addr_string);
  }
  return;
}

/**
 * read until socket closes, and then close our side
 */
#ifdef DEBUG
int LISA_READ_RESPONSE(lisa *l, char *s, int size, char * file, char * line) {
#else
int lisa_read_response(lisa *l, char *s, int size) {
#endif
	int received, i;
	received = 0;
  /* read until the connection is closed or an error*/
  for (i=0; i<30 && l->state != LISA_CLOSED && l->state != LISA_ERROR && l->state != LISA_TIMEOUT; i++) {
    received += lisa_recv(l, &(s[received]), size-received);
  }
  if (received >= 0) s[received] = 0;
  else s[0] = 0;
  lisa_close(l);
  COPY_MSG("lisa_read_response", "OK");
  return received;
}

void lisa_set_wait_time(lisa *l, int t) {
  l->wait_time.tv_sec = t;
}

