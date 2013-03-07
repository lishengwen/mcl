#include <net/mcl_event.h>
#include <net/mcl_net_raw.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#include "msg.h"

mcl_evbase *base = NULL;

void delete_cb(MCL_SOCKET sockfd, MCL_EVENT_TYPE type, void *udata)
{
	if (type == MCL_EV_DEL) {
		printf("close fd=%d\n", sockfd);
	}
	else {
		printf("error logic of backend, not delete! type = %d\n", type);
	}
	close(sockfd);
}

void error_cb(MCL_SOCKET sockfd, MCL_EVENT_TYPE type, void *udata)
{
	if (type != MCL_EV_EXP) {
		printf("error logic of backend, not error! type = %d\n", type);
	}
	else {
		printf("fd=%d on error\n", sockfd);
	}
	close(sockfd);
}

void read_cb(MCL_SOCKET sockfd, MCL_EVENT_TYPE type, void *udata)
{
	if (type != MCL_EV_READ) {
		printf("error logic of backend, not read! type = %d\n", type);
		return;
	}
	
	char buff[MAX_BUFF_LEN];
	int ret = mcl_recv(sockfd, buff, sizeof(msg_head), 0);

	if (ret > 0) {
		int val = -1;
		socklen_t len = sizeof(int);

		msg_head *head = (msg_head *)buff;
		int type = head->type;
		if (type == ECHO_STR) {
			int len = head->len;
			if (len > MAX_BUFF_LEN) {
				printf("too large packet from fd=%d\n", sockfd);
			}
			else if (len < sizeof(msg_head)) {
				perror("recv too small size");
			}
			else {
				ret = mcl_recv(sockfd, buff + sizeof(msg_head), len - sizeof(msg_head), MSG_WAITALL);
				buff[ret + sizeof(msg_head)] = '\0';
				printf("echo fd [%d] : %s\n", sockfd, buff + sizeof(msg_head));
			}
		}
		else if (type == EXIT) {
			mcl_event_loopexit(base);
		}
		else {
			printf("unkown command=%d\n", type);
		}
	}
	else {
		int val = -1;
		socklen_t len = sizeof(int);

		close(sockfd);
		mcl_event *ev = (mcl_event *)udata;
		printf("read_cb read event addr=%p\n", ev);

		//mcl_event_del(ev, base);
		if (ret < 0) {
			perror("error");
		}
	}
}

void accept_cb(MCL_SOCKET listenfd, MCL_EVENT_TYPE type, void *udata)
{
	if (type != MCL_EV_READ) {
		printf("error logic of backend, not read! type = %d\n", type);
		return;
	}
	int clientfd = mcl_accept(listenfd, NULL);
	if (!clientfd) {
		perror("accept");
		return;
	}
	printf("accept fd=%d\n", clientfd);
	mcl_event *rdev = mcl_event_alloc();

	if (!rdev) {
		perror("alloc event");
		return;
	}

	printf("got read event addr=%p\n", rdev);
	mcl_event_set(rdev, clientfd, MCL_EV_READ | MCL_EV_PERSIST, read_cb, delete_cb, error_cb, (void *)rdev, base);
	int ret = mcl_event_add(rdev, base);
	if (!ret) {
		printf("add fail [%d]!!\n", clientfd);
	}
}


int main(int argc, char *argv[])
{
	int ret = 0;
	base = mcl_evbase_new();
	printf("use backend: %s\n", mcl_evbase_name(base));

	mcl_sockaddr addr;
	mcl_init_sockaddr(NULL, 4444, AF_INET, &addr);

	MCL_SOCKET listenfd = mcl_socket(AF_INET, SOCK_STREAM, 0);
	printf("listen fd=%d\n", listenfd);
	mcl_sock_nonblock(listenfd);
	ret = mcl_bind(listenfd, &addr);
	if (!ret) {
		perror("bind");
		return 0;
	}
	ret = mcl_listen(listenfd, 0);
	if (!ret) {
		perror("listen");
		return 0;
	}

	mcl_event *listen_event = mcl_event_alloc();
	if (!listen_event) {
		perror("alloc event");
		return 0;
	}

	mcl_event_set(listen_event, listenfd, MCL_EV_READ | MCL_EV_PERSIST, 
			accept_cb, delete_cb, error_cb, (void *)listen_event, base);
	ret = mcl_event_add(listen_event, base);
	if (!ret) {
		printf("add fail [%d]!!\n", listenfd);
		return 0;
	}

	struct timeval tval;
	tval.tv_sec = 4;
	tval.tv_usec = 0;

	mcl_event_loop(&tval, base);

	mcl_evbase_destroy(base);
}

