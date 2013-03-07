#ifndef __MSG_H__
#define __MSG_H__

#define MAX_BUFF_LEN (1024)

typedef enum {
	ECHO_STR = 1,
	EXIT,
} msg_type;

typedef struct {
	int type;
	int len;
} msg_head;

#endif

