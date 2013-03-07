#ifndef __MCL_EVENT_H__
#define __MCL_EVENT_H__

#include "mcl_net_raw.h"

#ifdef _SYS_FREEBSD
#define _HAVE_KQUEUE_
#define _HAVE_SELECT_
#endif

#ifdef _LINUX_LESS_2_4
#define _HAVE_SELECT_
#define _HAVE_POLL_
#endif

#ifdef _LINUX_2_6
#define _HAVE_EPOLL_
#define _HAVE_SELECT_
#define _HAVE_POLL_
#endif

typedef enum { 
	MCL_EV_READ = 1, 
	MCL_EV_WRITE, 
	MCL_EV_EXP,
	MCL_EV_PERSIST,
	MCL_EV_ET, // default is LT
	MCL_EV_DEL,
} MCL_EVENT_TYPE;

typedef void (*mcl_event_cb)(MCL_SOCKET sockfd, MCL_EVENT_TYPE evtype, void *udata);

typedef struct {
	// The event identifier socket descriptor
	MCL_SOCKET sockfd;
	// The event type
	MCL_EVENT_TYPE ev_type;
	// The callback when event happens
	mcl_event_cb callback;
	// The callback when event is deleted
	mcl_event_cb cleanup_cb;
	// The callback when on error
	mcl_event_cb error_cb;
	// The user args which will pass to callback
	void *uargs;
} mcl_event;

#define MCL_EV_FD(_ev) (_ev->sockfd)
#define MCL_EV_TYPE(_ev) (_ev->ev_type)
#define MCL_EV_CB(_ev) _ev->callback
#define MCL_EV_CLEAN_CB(_ev) (_ev->cleanup_cb)
#define MCL_EV_ERROR_CB(_ev) (_ev->error_cb)
#define MCL_EV_UDATA(_ev) (_ev->uargs)

#define MCL_EVENT_INITIALIZER(_sockfd, _type, _cb, _clean_cb, _udata) \
	{_sockfd, _type, _cb, _clean_cb, _udata}

struct mcl_evbase_s;

typedef struct {
	// The name of evop
	const char *evop_name;
	// The init function. It should return a structure which holds things 
	// backend might use, and it will be set to base.evdb also
	// It return NULL on error
	void *(*init)(struct mcl_evbase_s *base);
	// The function which is used to add event to base
	int (*add)(mcl_event *ev, struct mcl_evbase_s *base);
	// The function which is used to delete event from base according to memory address
	int (*del)(mcl_event *ev, struct mcl_evbase_s *base);
	// The function which waits event forever or tval
	int (*dispatch)(struct timeval *tval, struct mcl_evbase_s *base);
	// The function which cleans all events inside backend
	void (*cleanup)(struct mcl_evbase_s *base);
} mcl_evops;

typedef struct mcl_evbase_s {
	// The event handle operation structure
	const mcl_evops *evop;
	// The event handle module data
	void *evdb;
	// The total event number
	int nevents;
	// The terminate flag which indicates ending event loop 
	// after one dispatch
	int term_flag;
	// The event loop timeout
	struct timeval tval;
} mcl_evbase;

mcl_evbase *mcl_evbase_new();
const char *mcl_evbase_name(mcl_evbase *base);
void mcl_evbase_destroy(mcl_evbase *base);
void mcl_event_loop(struct timeval *tval, mcl_evbase *base);

mcl_event *mcl_event_alloc();
// Allocate an event on Heap and initialize it
mcl_event *mcl_event_new(MCL_SOCKET sockfd, MCL_EVENT_TYPE type, 
		mcl_event_cb callback, mcl_event_cb cleanup_cb, mcl_event_cb error_cb, void *uargs, mcl_evbase *base);
// A ease of initializing an event
mcl_event *mcl_event_set(mcl_event *event, MCL_SOCKET sockfd, MCL_EVENT_TYPE type, 
		mcl_event_cb callback, mcl_event_cb cleanup_cb, mcl_event_cb error_cb, void *uargs, mcl_evbase *base);
// Add event to backend. The event MUST allocated on Heap, 
// cause event structure will be kept and used by backend
int mcl_event_add(mcl_event *event, mcl_evbase *base);
// Del event from backend. The deletion is according to sockfd
// value, so event can be stack varrient. And the event allocated
// on heap given by mcl_event_add will be free
int mcl_event_del(mcl_event *event, mcl_evbase *base);

void mcl_event_loopexit(mcl_evbase *base);

#endif

