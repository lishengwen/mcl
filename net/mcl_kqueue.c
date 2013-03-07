#include "mcl_event.h"
#include <stdio.h>

#ifdef _HAVE_KQUEUE_

#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>

#include <util/mcl_hash.h>

static void *kq_init(mcl_evbase *base);
static int kq_add(mcl_event *ev, mcl_evbase *base);
static int kq_del(mcl_event *ev, mcl_evbase *base);
static int kq_dispatch(struct timeval *tval, mcl_evbase *base);
static void kq_cleanup(mcl_evbase *base);

typedef struct {
	// Keeps kqueue() return value
	int kq;
	// The event set that maps sockfd to event
	mcl_hash *ht_rdevset;
	mcl_hash *ht_wrevset;
} kq_evdb;

typedef struct {
	mcl_event *ev;
} kq_ht_node;

const mcl_evops kqueue_ops = {
	"kqueue",
	kq_init,
	kq_add,
	kq_del,
	kq_dispatch,
	kq_cleanup,
};

static int MAX_EVENT_COUNT = 5000;

static void *kq_init(mcl_evbase *base)
{
	kq_evdb *db = (kq_evdb *)malloc(sizeof(*db));

	if (!db) return NULL;

	db->kq = kqueue();
	if (db->kq < 0) {
		goto err;
	}

	db->ht_rdevset = mcl_int_hash_new();
	db->ht_wrevset = mcl_int_hash_new();

	return (db);

err:
	free(db);
	return NULL;
}

static void event_ht_free(void *data)
{
	mcl_event *ev= (mcl_event *)data;

	if (MCL_EV_CLEAN_CB(ev)) {
		MCL_EV_CLEAN_CB(ev)(MCL_EV_FD(ev), MCL_EV_DEL, MCL_EV_UDATA(ev));
	}

	free(ev);
}

static void add_or_del_event(mcl_event *ev, mcl_hash *ht, int is_add)
{
	int sockfd = MCL_EV_FD(ev);
	if (is_add) {
		mcl_int_hash_insert_safe(sockfd, ev, ht);
	}
	else {
		mcl_int_hash_delete(sockfd, ht, event_ht_free);
	}
}

static void add_event(mcl_event *ev, kq_evdb *db)
{
	if (MCL_EV_TYPE(ev) == MCL_EV_READ) { 
		add_or_del_event(ev, db->ht_rdevset, 1);
	}
	else {
		add_or_del_event(ev, db->ht_wrevset, 1);
	}
}

static void del_event(mcl_event *ev, kq_evdb *db)
{
	if (MCL_EV_TYPE(ev) == MCL_EV_READ) { 
		add_or_del_event(ev, db->ht_rdevset, 0);
	}
	else {
		add_or_del_event(ev, db->ht_wrevset, 0);
	}
}

static int kq_add(mcl_event *ev, mcl_evbase *base)
{
	kq_evdb *db = NULL;
	struct kevent changes[1];
	int filter = 0;
	int ret = 0;

	MCL_IF_NOT_RET(ev, 0);
	MCL_IF_NOT_RET(base, 0);
	MCL_IF_NOT_RET(base->evdb, 0);

	filter = MCL_EV_TYPE(ev) & MCL_EV_READ ? 
		EVFILT_READ : EVFILT_WRITE;
	EV_SET(&changes[0], MCL_EV_FD(ev), filter, EV_ADD, 0, 0, (void *)ev);

	db = (kq_evdb *)base->evdb;
	ret = kevent(db->kq, changes, 1, NULL, 0, NULL);

	if (ret == -1) {
		return 0;
	}

	add_event(ev, db);

	return 1;
}

static int kq_del(mcl_event *ev, mcl_evbase *base)
{
	kq_evdb *db = NULL;
	struct kevent changes[1];
	int filter = 0;
	int ret = 0;

	MCL_IF_NOT_RET(ev, 0);
	MCL_IF_NOT_RET(base, 0);
	MCL_IF_NOT_RET(base->evdb, 0);

	filter = MCL_EV_TYPE(ev) & MCL_EV_READ ? 
		EVFILT_READ : EVFILT_WRITE;
	EV_SET(&changes[0], MCL_EV_FD(ev), filter, EV_DELETE, 0, 0, NULL);

	db = (kq_evdb *)base->evdb;
	kevent(db->kq, changes, 1, NULL, 0, NULL);

	if (MCL_EV_CLEAN_CB(ev)) {
		MCL_EV_CLEAN_CB(ev)(MCL_EV_FD(ev), MCL_EV_DEL, MCL_EV_UDATA(ev));
	}

	del_event(ev, db);

	return 1;
}

static void handle_events(struct kevent *events, int nevents, mcl_evbase *base)
{
	int i = 0;
	int filter = 0;
	int sockfd = 0;
	int flags = 0;
	mcl_event *ev = NULL;

	// No event happens
	if (nevents <= 0) return;

	for (i = 0; i < nevents; ++ i) {
		// Traverse all events
		sockfd = events[i].ident;
		filter = events[i].filter;
		flags = events[i].flags;
		if (events[i].udata) {
			ev = (mcl_event *)events[i].udata;
			if (flags & EV_ERROR) {
				//MCL_EV_CB(ev)(sockfd, MCL_EV_EXP, MCL_EV_UDATA(ev));
				if (MCL_EV_ERROR_CB(ev)) {
					MCL_EV_ERROR_CB(ev)(sockfd, MCL_EV_EXP, MCL_EV_UDATA(ev));
				}
			}
			else if (filter & EVFILT_READ) {
				if (MCL_EV_CB(ev)) { 
					MCL_EV_CB(ev)(sockfd, MCL_EV_READ, MCL_EV_UDATA(ev));
				}
			}
			else if (filter & EVFILT_WRITE) {
				if (MCL_EV_CB(ev)) {
					MCL_EV_CB(ev)(sockfd, MCL_EV_WRITE, MCL_EV_UDATA(ev));
				}
			}
			else {
				// No support for other conditions
				continue;
			}
		}

		if (!(MCL_EV_TYPE(ev) & MCL_EV_PERSIST)) {
			// Not persist event, delete it from backend
			kq_del(ev, base);
		}
	}
}

static int kq_dispatch(struct timeval *tval, mcl_evbase *base)
{
	struct kevent events[MAX_EVENT_COUNT];
	struct timespec tspec;
	struct timespec *tspecptr = NULL;
	kq_evdb *db = NULL;
	int nevents = 0;

	MCL_IF_NOT_RET(base, 0);
	// Not yet initilized
	MCL_IF_NOT_RET(base->evdb, 0);

	db = (kq_evdb *)base->evdb;

	if (tval) {
		base->tval.tv_sec = tval->tv_sec;
		base->tval.tv_usec = tval->tv_usec;

		tspec.tv_sec = tval->tv_sec;
		// microseconds --> nanoseconds
		tspec.tv_nsec = tval->tv_usec * 1000;
		tspecptr = &tspec;
	}

	while (1) {
		if (base->term_flag) {
			// The flag that indicates termination of
			// event loop is set, so return
			return 1;
		}

		nevents = kevent(db->kq, 
				NULL, 
				0, 
				events, 
				MAX_EVENT_COUNT, 
				tspecptr);

		if (nevents < 0) {
			// Error accurred
			return 0;
		}
		else {
			handle_events(events, nevents, base);
		}
	}
}

static void kq_cleanup(mcl_evbase *base)
{
	kq_evdb *db = (kq_evdb *)base->evdb;
	if (!db) {
		return;
	}

	mcl_hash_destroy(db->ht_rdevset, event_ht_free);
	mcl_hash_destroy(db->ht_wrevset, event_ht_free);

	free(db);
}

#endif

