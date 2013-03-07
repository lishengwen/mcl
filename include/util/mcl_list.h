#ifndef __MCL_LIST_H__
#define __MCL_LIST_H__

#include <stddef.h>
#include "mcl_iterator.h"
#include "mcl_common.h"

MCL_HEADER_BEGIN

typedef struct mcl_list_head_s {
	struct mcl_list_head_s *_prev, *_next;
} mcl_list_head;

typedef struct mcl_list_node_s {
	mcl_list_head _entry;
	void *_data;
} mcl_list_node;

typedef struct mcl_list_s {
	mcl_list_head _entries;
	int _list_sz;
	// TODO support mem pool

	void *(*iter_head)(mcl_iter *iter, struct mcl_list_s *container);
	void *(*iter_tail)(mcl_iter *iter, struct mcl_list_s *container);
	void *(*iter_next)(mcl_iter *iter, struct mcl_list_s *container);
	void *(*iter_prev)(mcl_iter *iter, struct mcl_list_s *container);
	void *(*iter_info)(mcl_iter *iter, struct mcl_list_s *containter);
	mcl_iter *(*iter_erase)(mcl_iter *iter, struct mcl_list_s *container);

	mcl_free_fn_t erase_fn;
	//void (*erase_fn)(void *data);

} mcl_list;

#define CONTAINER_OF(ptr, type, member) ({\
		const typeof(((type *)0)->member) *_tmp = (ptr); \
		(type *)(_tmp - offsetof(type, member)); \
	})

// ptr mcl_list_head指针
// type 包含该指针的类型
// member 该指针在类型中的成员名
#define __LIST_ENTRY(ptr, type, member) CONTAINER_OF((ptr), type, member)

#define INIT_LIST_VAL(name) {&(name),&(name)}

#define INITIALIZE_LIST(entp) \
	do { \
		(entp)->_prev = entp; \
		(entp)->_next = entp; \
	} while(0)

/*
#define _LIST_INSERT(elem, p, n) do { \
	(elem)->_prev = p; \
	(elem)->_next = n; \
	(n)->_prev = elem; \
	(p)->_next = elem; \
} while(0)
*/

//#define LIST_INSERT(elem, head) _LIST_INSERT((elem), (head), (head)->_next)

//#define LIST_INSERT_TAIL(elem, head) _LIST_INSERT((elem), (head)->_prev, (head))

static inline void _LIST_INSERT(mcl_list_head *elem, mcl_list_head *p, mcl_list_head *n)
{
	elem->_prev = p;
	elem->_next = n;
	n->_prev = elem;
	p->_next = elem;
}

static inline void LIST_INSERT(mcl_list_head *elem, mcl_list_head *head)
{
	_LIST_INSERT(elem, head, head->_next);
}

static inline void LIST_INSERT_TAIL(mcl_list_head *elem, mcl_list_head *head)
{
	_LIST_INSERT(elem, head->_prev, head);
}

#define IS_LIST_EMPTY(head) (head == head->_next)

#define MCL_LIST_MAX_SIZE	(1024 * 1024)

typedef int (*mcl_list_cmp_func)(void *src_val, void *dst_val);

mcl_list *mcl_list_new();
// destroy {@param list_ptr} totally
void mcl_list_destroy(mcl_list *list_ptr, mcl_free_fn_t free_fn);
int mcl_list_insert(void *data, mcl_list *list_ptr);
int mcl_list_insert_tail(void *data, mcl_list *list_ptr);
// delete elem whose addr equals to 
// {@param data} from {@param list_ptr}
// and will free data if free_fn is NOT NULL
int mcl_list_delete(void *data, mcl_list *list_ptr, mcl_free_fn_t free_fn);
int mcl_list_del_cmp(void *data, mcl_list *list_ptr, mcl_list_cmp_func cmp_func, mcl_free_fn_t free_fn);
/*
int mcl_list_remove(void *data, mcl_list *list_ptr);
int mcl_list_rm_cmp(void *data, mcl_list *list_ptr, mcl_list_cmp_func cmp_func);
*/

void *mcl_list_find(void *data, mcl_list *list_ptr, mcl_list_cmp_func cmp_func);

int mcl_list_size(mcl_list *list_ptr);

int mcl_list_ctrl(mcl_ds_ctrl_cmd cmd, void *data, mcl_list *list_ptr);

MCL_HEADER_END

#endif
