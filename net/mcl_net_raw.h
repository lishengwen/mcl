#ifndef __MCL_NET_RAW_H__
#define __MCL_NET_RAW_H__

/*
 * mcl raw net API. wrap posix socket API simply.
 * All API return 1 on success, 0/-1 on error
 */

#if defined(_SYS_FREEBSD) || defined(_SYS_LINUX)

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
#include <sys/time.h>

#include <util/mcl_string.h>
#include <util/mcl_log.h>

typedef int MCL_SOCKET;
typedef int MCL_FD;

typedef struct {
	struct sockaddr_in _addr;
	socklen_t _len;
} mcl_sockaddr;

#define MCL_SOCKADDR_IN(_mcl_addr_p) (_mcl_addr_p->_addr)
#define MCL_SOCKADDR_LEN(_mcl_addr_p) (_mcl_addr_p->_len)

typedef struct {
	MCL_SOCKET _sockfd;
	
	mcl_sockaddr _local;
	mcl_sockaddr _remote;
} mcl_endpoint;

int mcl_init_sockaddr(const char *ip, in_port_t port, sa_family_t family, mcl_sockaddr *addr);

MCL_SOCKET mcl_socket(int domain, int type, int protocol);
int mcl_shutdown_rd(MCL_SOCKET sockfd);
int mcl_shutdown_wr(MCL_SOCKET sockfd);
int mcl_shutdown_rw(MCL_SOCKET sockfd);

int mcl_bind(MCL_SOCKET sockfd, mcl_sockaddr *addr);
// @param timeout microseconds
int mcl_connect(MCL_SOCKET sockfd, mcl_sockaddr *addr, int timeout);
int mcl_listen(MCL_SOCKET sockfd, int backlog);
int mcl_accept(MCL_SOCKET sockfd, mcl_sockaddr *addr);
// @return 1 if all buff data sent; 0 on error
int mcl_send(MCL_SOCKET sockfd, const void *buff, size_t bufflen, int flags);
// @return length of message in bytes, 0 if no messages are available and peer has done an orderly shutdown, or -1 on error
ssize_t mcl_recv(MCL_SOCKET sockfd, void *buff, size_t bufflen, int flags);
int mcl_setsocketopt(MCL_SOCKET sockfd, int level, int option, const void *val, socklen_t len);

int mcl_sock_nonblock(MCL_SOCKET sockfd);

#else
// TODO MS_WINDOWS 
!system type not supported!
#endif // defined(_SYS_FREEBSD) || defined(_SYS_LINUX)

#endif // __MCL_NET_RAW_H__

