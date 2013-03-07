#include <net/mcl_event.h>
#include <stdio.h>
#include <errno.h>

#include "msg.h"

int main(int argc, char *argv[])
{
	mcl_sockaddr addr;
	mcl_init_sockaddr(NULL, 4444, AF_INET, &addr);
	MCL_SOCKET sockfd = mcl_socket(AF_INET, SOCK_STREAM, 0);

	int ret = mcl_connect(sockfd, &addr, 0);
	if (!ret) {
		perror("connect");
		return 0;
	}

	msg_head head;
	char *content = "hello world";

	head.type = ECHO_STR;
	head.len = sizeof(head) + strlen(content);

	ret = mcl_send(sockfd, (char *)&head, sizeof(head), 0);
	if (!ret) {
		perror("read");
		return 0;
	}
	ret = mcl_send(sockfd, content, strlen(content), 0);
	if (!ret) {
		perror("read");
		return 0;
	}
}

