#include "mcl_event.h"
#include <mcl_common.h>
#include <stdlib.h>

#ifdef _HAVE_KQUEUE_
extern mcl_evops kqueue_ops;
#endif

#ifdef _HAVE_SELECT_
extern mcl_evops select_ops;
#endif

#ifdef _HAVE_EPOLL_
extern mcl_evops epoll_ops;
#endif

static const mcl_evops *ops[] = {

#ifdef _HAVE_SELECT_
	&select_ops,
#endif

#ifdef _HAVE_EPOLL_
	&epoll_ops,
#endif

#ifdef _HAVE_KQUEUE_
	&kqueue_ops,
#endif

	NULL,
};

mcl_evbase *mcl_evbase_new()
{
	int i = 0;
	const mcl_evops *op_ptr = NULL;
	mcl_evbase *base = (mcl_evbase *)malloc(sizeof(*base));

	if (!base) {
		goto err;
	}

	for (i = 0, op_ptr = ops[i] ;op_ptr != NULL; op_ptr = ops[++ i]);

	// Find the last backend
	op_ptr = ops[i - 1];
	if (op_ptr == NULL) return NULL;

	base->evop = op_ptr;
	//base->evop = &select_ops;
	base->nevents = 0;
	base->term_flag = 0;
	
	base->evdb = base->evop->init(base);
	if (!base->evdb) {
		goto err;
	}

success:
	return base;
err:
	if (base) {
		free(base);
	}
	return NULL;
}

void mcl_evbase_destroy(mcl_evbase *base)
{
	if (base) {
		if (base->evop) {
			base->evop->cleanup(base);
		}
		free(base);
	}
}

void mcl_event_loop(struct timeval *tval, mcl_evbase *base)
{
	if (!base) {
		return;
	}

	if (!base->evop) {
		return;
	}

	base->evop->dispatch(tval, base);
}

mcl_event *mcl_event_new(MCL_SOCKET sockfd, MCL_EVENT_TYPE type, 
		mcl_event_cb callback, mcl_event_cb cleanup_cb, mcl_event_cb error_cb, void *uargs, mcl_evbase *base)
{
	mcl_event *ev = (mcl_event *)malloc(sizeof(*ev));
	mcl_event_set(ev, sockfd, type, callback, cleanup_cb, error_cb, uargs, base);
}

mcl_event *mcl_event_alloc()
{
	mcl_event *ev = (mcl_event *)malloc(sizeof(*ev));

	if (ev) return ev;
	return NULL;
}

mcl_event *mcl_event_set(mcl_event *event, MCL_SOCKET sockfd, MCL_EVENT_TYPE type, 
		mcl_event_cb callback, mcl_event_cb cleanup_cb, mcl_event_cb error_cb, void *uargs, mcl_evbase *base)
{
	MCL_IF_NOT_RET(event, NULL);
	MCL_IF_NOT_RET(base, NULL);

	event->sockfd = sockfd;
	event->ev_type = type;
	event->callback = callback;
	event->cleanup_cb = cleanup_cb;
	event->error_cb = error_cb;
	
	event->uargs = uargs;

	return event;
}

int mcl_event_add(mcl_event *event, mcl_evbase *base)
{
	MCL_IF_NOT_RET(event, 0);
	MCL_IF_NOT_RET(base, 0);
	MCL_IF_NOT_RET(base->evop, 0);

	return base->evop->add(event, base);
}

int mcl_event_del(mcl_event *event, mcl_evbase *base)
{
	MCL_IF_NOT_RET(event, 0);
	MCL_IF_NOT_RET(base, 0);
	MCL_IF_NOT_RET(base->evop, 0);

	return base->evop->del(event, base);
}

const char *mcl_evbase_name(mcl_evbase *base)
{
	return base->evop->evop_name;
}

void mcl_event_loopexit(mcl_evbase *base)
{
	base->term_flag = 1;	
}

