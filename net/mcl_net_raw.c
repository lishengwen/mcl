#include "mcl_net_raw.h"
#include <unistd.h>
#include <fcntl.h>

#define CONNECT_RETRY_US (10 * 1000)
#define LISTEN_BACKLOG	 (50)

#if defined(_SYS_FREEBSD) || defined(_SYS_LINUX)

int mcl_init_sockaddr(const char *ip, in_port_t port, sa_family_t family, mcl_sockaddr *addr)
{
	MCL_SOCKADDR_IN(addr).sin_family = family;
	MCL_SOCKADDR_IN(addr).sin_port = htons(port);
	bzero(&(MCL_SOCKADDR_IN(addr).sin_zero), sizeof(MCL_SOCKADDR_IN(addr).sin_zero));

	if (ip == NULL) {
		MCL_SOCKADDR_IN(addr).sin_addr.s_addr = INADDR_ANY;
	}
	else {
		if (inet_pton(AF_INET, ip, (void *)&(MCL_SOCKADDR_IN(addr).sin_addr)) <= 0) {
			return 0;
		}
	}

	MCL_SOCKADDR_LEN(addr) = sizeof(MCL_SOCKADDR_IN(addr));	

	return 1;
}

MCL_SOCKET mcl_socket(int domain, int type, int protocol)
{
	MCL_SOCKET sockfd = socket(domain, type, protocol);
	if (sockfd < 0) {
		return 0;
	}

	return sockfd;
}

static int shutdown_socket(MCL_SOCKET sockfd, int how)
{
	int ret = shutdown(sockfd, how);
	if (ret < 0) {
		return 0;
	}

	return 1;
}

int mcl_shutdown_rd(MCL_SOCKET sockfd)
{
	return shutdown_socket(sockfd, SHUT_RD);	
}

int mcl_shutdown_wr(MCL_SOCKET sockfd)
{
	return shutdown_socket(sockfd, SHUT_WR);	
}

int mcl_shutdown_rw(MCL_SOCKET sockfd)
{
	return shutdown_socket(sockfd, SHUT_RDWR);	
}

int mcl_bind(MCL_SOCKET sockfd, mcl_sockaddr *addr)
{
	socklen_t len = MCL_SOCKADDR_LEN(addr);
	int ret = bind(sockfd, (struct sockaddr *)&(MCL_SOCKADDR_IN(addr)), len);

	if (ret < 0) {
		return 0;
	}

	return 1;
}

int mcl_connect(MCL_SOCKET sockfd, mcl_sockaddr *addr, int timeout)
{
	socklen_t len = MCL_SOCKADDR_LEN(addr);
	int ret = connect(sockfd, (struct sockaddr *)&(MCL_SOCKADDR_IN(addr)), len);

	if (ret < 0) {
		while (timeout > 0) {
			usleep(CONNECT_RETRY_US);
			timeout -= CONNECT_RETRY_US;
			ret = connect(sockfd, (struct sockaddr *)&(MCL_SOCKADDR_IN(addr)), len);
			if (ret == 0) {
				return 1;
			}
		}

		return 0;
	}
	else {
		return 1;
	}
}

int mcl_listen(MCL_SOCKET sockfd, int backlog)
{
	int ret = 0;

	if (backlog <= 0) {
		backlog = LISTEN_BACKLOG;
	}

	ret = listen(sockfd, backlog);	

	if (ret < 0) {
		return 0;
	}

	return 1;
}

int mcl_accept(MCL_SOCKET sockfd, mcl_sockaddr *addr)
{
	struct sockaddr_in accepted_addr;
	socklen_t accepted_len;

	int ret = accept(sockfd, (struct sockaddr *)&accepted_addr, &accepted_len);
	if (ret > 0) {
		if (addr) {
			MCL_SOCKADDR_IN(addr) = accepted_addr;
			MCL_SOCKADDR_LEN(addr) = accepted_len;
		}

		return ret;
	}
	else {
		return 0;
	}
}

int mcl_send(MCL_SOCKET sockfd, const void *buff, size_t bufflen, int flags)
{
	size_t to_sent = bufflen;
	ssize_t sent = 0;	
	while (to_sent > 0) {
		sent = send(sockfd, buff, bufflen, flags);
		if (sent < 0) {
			return 0;
		}
		to_sent -= sent;
	}

	return bufflen;
}

ssize_t mcl_recv(MCL_SOCKET sockfd, void *buff, size_t bufflen, int flags)
{
	return recv(sockfd, buff, bufflen, flags);
}

int mcl_setsocketopt(MCL_SOCKET sockfd, int level, int option, const void *val, socklen_t len)
{
	int ret = setsockopt(sockfd, level, option, val, len);
	if (ret < 0) {
		return 0;
	}

	return 1;
}

int mcl_sock_nonblock(MCL_SOCKET sockfd)
{
	int flags = fcntl(sockfd, F_GETFL, 0);
	if (flags < 0) {
		return 0;
	}

	int ret = fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
	if (ret < 0) {
		return 0;
	}

	return 1;
}

#else
// TODO MS_WINDOWS 
#endif

