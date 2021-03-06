#include <mcl_list.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#define INIT_DEFAULT_FUNC(_listp) \
	do { \
		_listp->iter_head = mcl_def_list_iter_head; \
		_listp->iter_tail = mcl_def_list_iter_tail; \
		_listp->iter_next = mcl_def_list_iter_next; \
		_listp->iter_prev = mcl_def_list_iter_prev; \
		_listp->iter_info = mcl_def_list_iter_info; \
		_listp->iter_erase = mcl_def_list_iter_erase; \
	} while (0)

#define MCL_LIST_ENTRY(_entp) __LIST_ENTRY((_entp), mcl_list_node, _entry)

#define POINTER_RET_NULL(_ptr) \
	do { \
		if (_ptr == NULL) { \
			return NULL; \
		} \
	} while(0)

static inline int list_del(void *data, mcl_list *list_ptr, int free_flag, mcl_list_cmp_func cmp_func, mcl_free_fn_t free_fn);

static inline void common_iter_op(mcl_iter *iter)
{
	iter->_u._index = -1;
	iter->_sz = -1;
}

static void *mcl_def_list_iter_head(mcl_iter *iter, mcl_list *list)
{
	mcl_list_head *entries = NULL;

	POINTER_RET_NULL(iter);
	POINTER_RET_NULL(list);

	common_iter_op(iter);
	entries = &list->_entries;

	if (!IS_LIST_EMPTY(entries)) {
		iter->_ptr = (void *)MCL_LIST_ENTRY(entries->_next);
		iter->_data = (void *)entries->_next;
	}
	else {
		iter->_ptr = NULL;
		iter->_data = NULL;
	}

	return iter->_ptr;
}

static void *mcl_def_list_iter_tail(mcl_iter *iter, mcl_list *list)
{
	mcl_list_head *entries = NULL;

	POINTER_RET_NULL(iter);
	POINTER_RET_NULL(list);

	common_iter_op(iter);
	entries = &list->_entries;
	
	if (!IS_LIST_EMPTY(entries)) {
		iter->_ptr = (void *)MCL_LIST_ENTRY(entries->_prev);
		iter->_data = (void *)entries->_prev;
	}
	else {
		iter->_ptr = NULL;
		iter->_data = NULL;
	}

	return iter->_ptr;
}

static void *mcl_def_list_iter_next(mcl_iter *iter, mcl_list *list)
{
	mcl_list_head *entries = NULL;
	mcl_list_head *ent = NULL;

	POINTER_RET_NULL(iter);
	POINTER_RET_NULL(list);

	common_iter_op(iter);
	entries = &list->_entries;

	if (!IS_LIST_EMPTY(entries)) {
		ent = (mcl_list_head *)iter->_data;

		if (!ent || ent->_next == entries) {
			// last elem alrdy
			iter->_ptr = NULL;
			iter->_data = NULL;
			return NULL;
		}
		else {
			iter->_ptr = (void *)MCL_LIST_ENTRY(ent->_next);
			iter->_data = (void *)ent->_next;
			return iter->_ptr;
		}
	}
	else {
		iter->_ptr = NULL;
		iter->_data = NULL;
		return NULL;
	}
}

static void *mcl_def_list_iter_prev(mcl_iter *iter, mcl_list *list)
{
	mcl_list_head *entries = NULL;
	mcl_list_head *ent = NULL;

	POINTER_RET_NULL(iter);
	POINTER_RET_NULL(list);

	common_iter_op(iter);
	entries = &list->_entries;

	if (!IS_LIST_EMPTY(entries)) {
		ent = (mcl_list_head *)iter->_data;

		if (!ent || ent->_prev == entries) {
			// first elem alrdy
			iter->_ptr = NULL;
			iter->_data = NULL;
			return NULL;
		}
		else {
			iter->_ptr = (void *)MCL_LIST_ENTRY(ent->_prev);
			iter->_data = (void *)ent->_prev;
			return iter->_ptr;
		}
	}
	else {
		iter->_ptr = NULL;
		iter->_data = NULL;
		return NULL;
	}
}

static void *mcl_def_list_iter_info(mcl_iter *iter, mcl_list *list)
{
	mcl_list_head *ent = NULL;
	mcl_list_node *node = NULL;

	POINTER_RET_NULL(iter);
	POINTER_RET_NULL(list);

	ent = (mcl_list_head *)iter->_data;	
	if (!ent) {
		return NULL;
	}
	
	node = (mcl_list_node *)iter->_ptr;
	if (!node) {
		return NULL;
	}

	assert(node == MCL_LIST_ENTRY(ent));

	return node->_data;
}

static mcl_iter *mcl_def_list_iter_erase(mcl_iter *iter, mcl_list *list)
{
	mcl_list_head *ent = NULL;
	mcl_list_node *node = NULL;
	void *data = NULL;
	
	POINTER_RET_NULL(iter);
	POINTER_RET_NULL(list);

	ent = (mcl_list_head *)iter->_data;
	if (!ent) {
		return NULL;
	}

	node = (mcl_list_node *)iter->_ptr;	
	if (!node) {
		return NULL;
	}

	// ensure data is right
	assert(node == MCL_LIST_ENTRY(ent));
	data = node->_data;

	// move iterator to next posistion
	list->iter_next(iter, list);

	// delete node from list
	if (list->erase_fn) {
		list_del(data, list, 1, NULL, list->erase_fn);
	}
	else {
		list_del(data, list, 0, NULL, NULL);
	}

	return iter;
}

mcl_list *mcl_list_new()
{
	mcl_list *listp = (mcl_list *)malloc(sizeof(*listp));
	listp->_list_sz = 0;
	listp->erase_fn = NULL;

	INITIALIZE_LIST(&(listp->_entries));

	INIT_DEFAULT_FUNC(listp);

	return listp;
}

void mcl_list_destroy(mcl_list *list_ptr, mcl_free_fn_t free_fn)
{
	mcl_list_head *ent = NULL;
	mcl_list_node *node = NULL;
	mcl_list_head *entries = &list_ptr->_entries;

	if (!IS_LIST_EMPTY(entries)) {
		for (ent = entries->_next; ent != entries;) {
			node = (mcl_list_node *)MCL_LIST_ENTRY(ent);
			if (node->_data && free_fn) {
				free_fn(node->_data);
			}
			ent = ent->_next;
			free(node);
		}
	}
	else {
		fprintf(stderr, "warning: destroy empty list\n");
	}

	free(list_ptr);
}

int mcl_list_insert(void *data, mcl_list *list_ptr)
{
	if (list_ptr->_list_sz >= MCL_LIST_MAX_SIZE) {
		fprintf(stderr, "list [%p] size beyond max size [%d]", list_ptr, MCL_LIST_MAX_SIZE);
		return 0;
	}

	mcl_list_node *node = (mcl_list_node *)malloc(sizeof(*node));

	node->_data = data;
	LIST_INSERT(&node->_entry, &list_ptr->_entries);
	++ list_ptr->_list_sz;

	return 1;
}

int mcl_list_insert_tail(void *data, mcl_list *list_ptr)
{
	if (list_ptr->_list_sz >= MCL_LIST_MAX_SIZE) {
		fprintf(stderr, "list [%p] size beyond max size [%d]", list_ptr, MCL_LIST_MAX_SIZE);
		return 0;
	}

	mcl_list_node *node = (mcl_list_node *)malloc(sizeof(*node));

	node->_data = data;
	LIST_INSERT_TAIL(&node->_entry, &list_ptr->_entries);
	++ list_ptr->_list_sz;

	return 1;
}

#define HANDLE_ELEM(ent, list_ptr, free_flag, free_fn) \
	do { \
		ent->_prev->_next = ent->_next; \
		ent->_next->_prev = ent->_prev; \
		if (free_flag && node->_data && free_fn) { \
			free_fn(node->_data); \
		} \
		free(node); \
		-- list_ptr->_list_sz; \
	} while(0)

static inline int list_del(void *data, mcl_list *list_ptr, int free_flag, mcl_list_cmp_func cmp_func, mcl_free_fn_t free_fn)
{
	mcl_list_head *ent = NULL;
	mcl_list_node *node = NULL;
	mcl_list_head *entries = &(list_ptr->_entries);

	if (!IS_LIST_EMPTY(entries)) {
		//printf("entries: %p, prev: %p, next: %p\n", entries, entries->_prev, entries->_next);
		for (ent = entries->_next; ent != entries; ent = ent->_next) {
			node = (mcl_list_node *)MCL_LIST_ENTRY(ent);
			//printf("node: %p, node_data: %p, data: %p, prev: %p, next: %p\n", node, node->_data, data, ent->_prev, ent->_next);
			if (cmp_func && cmp_func(data, node->_data)) {
				HANDLE_ELEM(ent, list_ptr, free_flag, free_fn);
				return 1;
			}
			else if (node->_data == data) {
				HANDLE_ELEM(ent, list_ptr, free_flag, free_fn);
				return 1;
			}
		}
	}
	return 0;
}

int mcl_list_delete(void *data, mcl_list *list_ptr, mcl_free_fn_t free_fn)
{
	return list_del(data, list_ptr, 1, NULL, free_fn);
}

int mcl_list_del_cmp(void *data, mcl_list *list_ptr, mcl_list_cmp_func cmp_func, mcl_free_fn_t free_fn)
{
	return list_del(data, list_ptr, 1, cmp_func, free_fn);
}

/*
int mcl_list_remove(void *data, mcl_list *list_ptr)
{
	return list_del(data, list_ptr, 0, NULL);
}

int mcl_list_rm_cmp(void *data, mcl_list *list_ptr, mcl_list_cmp_func cmp_func)
{
	return list_del(data, list_ptr, 0, cmp_func);
}
*/

void *mcl_list_find(void *data, mcl_list *list_ptr, mcl_list_cmp_func cmp_func)
{
	void *dst_data = NULL;

	MCL_IF_NOT_RET(list_ptr, NULL);
	MCL_IF_NOT_RET(data, NULL);

	mcl_iter it = MCL_ITER_INITIALIZER;
	MCL_FOREACH(it, list_ptr) {
		dst_data = MCL_ITERATOR_INFO(it, list_ptr);
		if (!dst_data) {
			return NULL;
		}
		if (cmp_func) {
			if (cmp_func(data, dst_data)) {
				return dst_data;
			}
		}
		else {
			if (data == dst_data) {
				return dst_data;
			}
		}
	}

	return NULL;
}

int mcl_list_size(mcl_list *list_ptr)
{
	if (!list_ptr) return 0;

	return list_ptr->_list_sz;
}

int mcl_list_ctrl(mcl_ds_ctrl_cmd cmd, void *data, mcl_list *list_ptr)
{
	mcl_free_fn_t free_fn = NULL;

	MCL_IF_NOT_RET(data, 0);
	MCL_IF_NOT_RET(list_ptr, 0);

	switch (cmd) {
		case MCL_REGIST_ERASE_FN:
			free_fn = (mcl_free_fn_t) data;
			list_ptr->erase_fn = free_fn;
			return 1;
		case MCL_UNREGIST_ERASE_FN:
			list_ptr->erase_fn = NULL;
			return 1;
		default:
			break;
	}

	return 0;
}

