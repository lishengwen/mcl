#ifndef __MCL_LIST_H__
#define __MCL_LIST_H__

#include <stddef.h>
#include <mcl_iterator.h>

typedef struct mcl_list_head_s {
	struct mcl_list_head_s *_prev, *_next;
} mcl_list_head;

typedef struct mcl_list_node_s {
	mcl_list_head _entry;
	void *_data;
} mcl_list_node;

typedef struct mcl_list_s {
	//mcl_list_head _list;
	mcl_list_head _entries;
	int _list_sz;

	void *(*iter_head)(mcl_iter *iter, mcl_list *container);
	void *(*iter_tail)(mcl_iter *iter, mcl_list *container);
	void *(*iter_next)(mcl_iter *iter, mcl_list *container);
	void *(*iter_prev)(mcl_lter *iter, mcl_list *container);
	mcl_list_node *(*iter_info)(mcl_iter *iter, mcl_list *containter);

} mcl_list;

#define CONTAINER_OF(ptr, type, member) ({\
		const typeof(((type *)0)->member) *_tmp = ptr; \
		(type *)(_tmp - offset(type, member)); \
	})

// ptr mcl_list_head指针
// type 包含该指针的类型
// member 该指针在类型中的成员名
#define LIST_ENTRY(ptr, type, member) CONTAINER_OF(ptr, type, member)

#define INIT_LIST(name) {&name,&name}

#define _LIST_INSERT(elem, p, n) do { \
	elem->_prev = p; \
	elem->_next = n; \
	p->_next = elem; \
	n->_prev = elem; \
} while(0)

#define LIST_INSERT(elem, head) _LIST_INSERT(elem, head, head->_next)

#define LIST_INSERT_TAIL(elem, head) _LIST_INSERT(elem, head->_prev, head)

#define IS_LIST_EMPTY(head) (head == head->_next)

#define MCL_LIST_MAX_SIZE	(1024 * 1024)

mcl_list *mcl_list_new();
void mcl_list_delete(mcl_list *list_ptr);
int mcl_list_insert(void *data, mcl_list *list_ptr);
int mcl_list_delete(void *data, mcl_list *list_ptr);

#endif
