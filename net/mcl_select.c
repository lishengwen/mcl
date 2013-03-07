#include "mcl_event.h"
#include <stdio.h>

#ifdef _HAVE_SELECT_

#include <sys/select.h>
#include <util/mcl_hash.h>

static void *select_init(mcl_evbase *base);
static void select_cleanup(mcl_evbase *base);
static int select_add(mcl_event *ev, mcl_evbase *base);
static int select_del(mcl_event *ev, mcl_evbase *base);
static int select_dispatch(struct timeval *tval, mcl_evbase *base);
static void event_ht_free(void *data);

typedef struct {
	// The hash table which maps sockfd to event
	mcl_hash *ht;
	// The fd_set structure which is passed to select function
	fd_set *fds;
} ev_set;

typedef struct {
	int n_maxfdn;

	ev_set *in_evs;
	ev_set *out_evs;
	ev_set *exp_evs;

} slt_evdb;

typedef struct {
	mcl_event *ev;
} slt_ht_node;

const mcl_evops select_ops = {
	"select",
	select_init,
	select_add,
	select_del,
	select_dispatch,
	select_cleanup,
};

inline static ev_set *malloc_evset()
{
	ev_set *evset = (ev_set *)malloc(sizeof(ev_set));
	MCL_IF_NOT_RET(evset, NULL);

	evset->ht = mcl_int_hash_new();
	MCL_IF_NOT_RET(evset->ht, NULL);
	mcl_hash_ctrl(MCL_REGIST_ERASE_FN, event_ht_free, evset->ht);

	evset->fds = (fd_set *)malloc(sizeof(fd_set));
	MCL_IF_NOT_RET(evset->fds, NULL);
	FD_ZERO(evset->fds);

	return evset;
}

inline static void free_evset(ev_set *evset)
{
	if (evset) {
		if (evset->ht) {
			mcl_hash_destroy(evset->ht, event_ht_free);
		}

		if (evset->fds) {
			free(evset->fds);
		}

		free(evset);
	}
}

static void *select_init(mcl_evbase *base)
{
	slt_evdb *db = (slt_evdb *)malloc(sizeof(*db));
	if (!db) {
		// out of memory
		return NULL;
	}

	db->in_evs = malloc_evset();
	db->out_evs = malloc_evset();
	db->exp_evs = malloc_evset();

	//printf("readfds addr=%p\n", db->in_evs->fds);
	
	MCL_IF_NOT_RET(db->in_evs, NULL);
	MCL_IF_NOT_RET(db->out_evs, NULL);
	MCL_IF_NOT_RET(db->exp_evs, NULL);

	db->n_maxfdn = 0;

	return (db);
}

static void event_ht_free(void *data)
{
	slt_ht_node *node = (slt_ht_node *)data;

	mcl_event *ev = node->ev;
	if (MCL_EV_CLEAN_CB(ev)) {
		MCL_EV_CLEAN_CB(ev)(MCL_EV_FD(ev), MCL_EV_DEL, MCL_EV_UDATA(ev));
	}

	//printf("delete event %p\n", ev);

	free(ev);

	free(node);
}

static void select_cleanup(mcl_evbase *base)
{
	slt_evdb *db = (slt_evdb *)base->evdb;
	if (!db) {
		return;
	}

	free_evset(db->in_evs);
	free_evset(db->out_evs);
	free_evset(db->exp_evs);

	free(db);
}

inline static ev_set *get_evset(mcl_event *ev, slt_evdb *db)
{
	ev_set *evset = NULL;

	if (ev->ev_type & MCL_EV_READ) {
		//printf("got readfds\n");
		evset = db->in_evs;
	}
	else if (ev->ev_type & MCL_EV_WRITE) {
		evset = db->out_evs;
	}
	else if (ev->ev_type & MCL_EV_EXP) {
		evset = db->exp_evs;
		return NULL;
	}

	return evset;
}

inline static int add_event(mcl_event *ev, ev_set *evset)
{
	slt_ht_node *node = NULL;
	int sockfd = ev->sockfd;
	int ht_ret = 0;
	if (sockfd < 0) {
		return 0;
	}
	node = (slt_ht_node *)malloc(sizeof(*node));
	node->ev = ev;
	ht_ret = mcl_int_hash_insert_safe(sockfd, (void *)node, evset->ht);
	if (!ht_ret) {
		free(node);
		//printf("insert hash fail!\n");
		return 0;
	}

	// FD_SET
	FD_SET(sockfd, evset->fds);
	//printf("set fd %d\n", sockfd);

	//printf("readfds addr=%p\n", evset->fds);

	//printf("add event %p, fd [%d]\n", ev, sockfd);

	return sockfd;
}

inline static int del_event(mcl_event *ev, ev_set *evset)
{
	int sockfd = ev->sockfd;
	int ht_ret = 0;
	if (sockfd < 0) {
		return 0;
	}

	if (!FD_ISSET(sockfd, evset->fds)) {
		// not in select queue
		return 0;
	}

	FD_CLR(sockfd, evset->fds);
	//printf("clean sockfd: %d\n", sockfd);

	return mcl_int_hash_delete(sockfd, evset->ht, event_ht_free);
}

inline static int add_expevent(mcl_event *ev, slt_evdb *db)
{
	ev_set *evset = db->exp_evs;
	return add_event(ev, evset);
}

inline static int del_expevent(mcl_event *ev, slt_evdb *db)
{
	ev_set *evset = db->exp_evs;
	return del_event(ev, evset);
}

static int event_add_or_del(mcl_event *ev, mcl_evbase *base, int is_add)
{
	ev_set *evset= NULL;
	slt_evdb *db = NULL;
	int ret = 0;

	MCL_IF_NOT_RET(ev, 0);
	MCL_IF_NOT_RET(base, 0);
	MCL_IF_NOT_RET(base->evdb, 0);

	db = (slt_evdb *)base->evdb;
	
	evset = get_evset(ev, db);
	//printf("evset addr=%p, handling ev=%p\n", evset, ev);
	MCL_IF_NOT_RET(evset, 0);

	if (is_add) {
		ret = add_event(ev, evset);
		if (ret) {
			db->n_maxfdn = db->n_maxfdn < ret ? ret : db->n_maxfdn;
			//printf("maxfd: %d\n", db->n_maxfdn);
			if (!(MCL_EV_TYPE(ev) & MCL_EV_EXP)) {
				// All event added in will be added to exception evset
				add_expevent(ev, db);
			}
			return 1;
		}
		return 0;	
	}
	else {
		ret = del_event(ev, evset);
		if (!(MCL_EV_TYPE(ev) & MCL_EV_EXP)) {
			del_expevent(ev, db);
		}
		return ret;	
	}
}

static int select_add(mcl_event *ev, mcl_evbase *base)
{
	return event_add_or_del(ev, base, 1);
}

static int select_del(mcl_event *ev, mcl_evbase *base)
{
	return event_add_or_del(ev, base, 0);
}

inline static int is_valid_timeval(struct timeval *tval)
{
	MCL_IF_NOT_RET(tval, 0);
	MCL_IF_NOT_RET((tval->tv_sec > 0 || tval->tv_usec > 0), 0);

	return 1;
}

static void handle_events(ev_set *evset, MCL_EVENT_TYPE type)
{
	mcl_iter it = MCL_ITER_INITIALIZER;
	int sockfd = 0;
	int err = 0;
	int *sockptr = NULL;
	slt_ht_node *valptr = NULL;
	mcl_event *ev = NULL;

	for (ITER_HEAD(&it, evset->ht); !ITER_NULL(&it);) {
		sockptr = (int *)mcl_hash_iter_key(&it);
		valptr = (slt_ht_node *)mcl_hash_iter_val(&it);
		sockfd = *sockptr;
		if (FD_ISSET(sockfd, evset->fds)) {
			ev = valptr->ev;
			if (type != MCL_EV_EXP) {
				if (MCL_EV_CB(ev)) {
					//printf("call event %p callback\n", ev);
					MCL_EV_CB(ev)(sockfd, type, MCL_EV_UDATA(ev));
				}
			}
			else {
				if (MCL_EV_ERROR_CB(ev)) {
					MCL_EV_ERROR_CB(ev)(sockfd, type, MCL_EV_UDATA(ev));
					err = 1;
				}
			}

			if (!(MCL_EV_TYPE(ev) & MCL_EV_PERSIST) || err) {
				// delete event
				MCL_ITERATOR_ERASE(it, evset->ht);
				FD_CLR(sockfd, evset->fds);
				//printf("dispatch: clean fd %d\n", sockfd);
				err = 0;
				continue;
			}
		}

		ITER_NEXT(&it, evset->ht);
	}
}

static void init_fds(ev_set *evset)
{
	mcl_hash *ht = evset->ht;
	fd_set *fds = evset->fds;
	mcl_iter it = MCL_ITER_INITIALIZER;
	int sockfd = 0;
	int *htkey = NULL;

	FD_ZERO(fds);

	MCL_FOREACH(it, ht) {
		htkey = mcl_hash_iter_key(&it);	
		sockfd = *htkey;
		FD_SET(sockfd, fds);
	}
}

static int select_dispatch(struct timeval *tval, mcl_evbase *base)
{
	int ret = 0;
	slt_evdb *db = NULL;
	mcl_iter it = MCL_ITER_INITIALIZER;
	int *sockfd = NULL;
	mcl_event *ev = NULL;
	struct timeval *valptr = NULL;

	MCL_IF_NOT_RET(base, 0);
	MCL_IF_NOT_RET(base->evdb, 0);

	if (tval) {
		base->tval.tv_sec = tval->tv_sec;
		base->tval.tv_usec = tval->tv_usec;
		valptr = &(base->tval);
	}

	db = (slt_evdb *)base->evdb;

	while (23) {
		if (base->term_flag) {
			// The flag that indicates termination of
			// event loop is set, so return
			return 1;
		}

		// Egg pain!
		init_fds(db->in_evs);
		init_fds(db->out_evs);
		init_fds(db->exp_evs);

		//printf("readfds addr=%p, max=%d\n", db->in_evs->fds, db->n_maxfdn);

		ret = select(db->n_maxfdn + 1, 
				db->in_evs->fds,
				db->out_evs->fds,
				db->exp_evs->fds,
				valptr);

		if (ret > 0) {
			printf("event!\n");
			// Something happen
			handle_events(db->in_evs, MCL_EV_READ);
			handle_events(db->out_evs, MCL_EV_WRITE);
			handle_events(db->exp_evs, MCL_EV_EXP);
		}
		else if (ret == 0) {
			printf("no event!\n");
			// No event happens
			continue;
		}
		else {
			printf("error!\n");
			// Error accurred
			return 0;
		}
	}
}

#endif

