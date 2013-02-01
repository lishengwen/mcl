#include <mcl_list.h>
#include <stdlib.h>
#include <stdio.h>

#define INIT_DEFAULT_FUNC(_listp) \
	do { \
		_listp->iter_head = mcl_def_list_iter_head; \
		_listp->iter_tail = mcl_def_list_iter_tail; \
		_listp->iter_next = mcl_def_list_iter_next; \
		_listp->iter_prev = mcl_def_list_iter_prev; \
		_listp->iter_info = mcl_def_list_iter_info; \
	} while (0)

#define MCL_LIST_ENTRY(_entp) LIST_ENTRY(_entp, mcl_list_node, _entry)

#define POINTER_RET_NULL(_ptr) \
	do { \
		if (_ptr == NULL) { \
			return NULL; \
		} \
	} while(0)

static inline void common_iter_op(mc_iter *iter)
{
	iter->_index = -1;
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
	}
	else {
		iter->_ptr = NULL;
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
	}
	else {
		iter->_ptr = NULL;
	}

	return iter->_ptr;
}

static void *mcl_def_list_iter_next(mcl_iter *iter, mcl_list *list)
{
	mcl_list_head *entries = NULL;
	mcl_list_node *node = NULL;
	mcl_list_head *ent = NULL;

	POINTER_RET_NULL(iter);
	POINTER_RET_NULL(list);

	common_iter_op(iter);
	entries = &list->_entries;

	if (!IS_LIST_EMPTY(entries)) {
		node = (mcl_list_node *)iter->ptr;	
		ent = &node->_entry;
		if (ent->_next == entries) {
			iter->ptr = NULL;
		}
		else {
			iter->_ptr = (void *)MCL_LIST_ENTRY(ent->_next);
		}
	}
	else {
		iter->_ptr = NULL;
	}

	return iter->_ptr;
}

static void *mcl_def_list_iter_prev(mcl_iter *iter, mcl_list *list)
{
	mcl_list_head *entries = NULL;
	mcl_list_node *node = NULL;
	mcl_list_head *ent = NULL;

	POINTER_RET_NULL(iter);
	POINTER_RET_NULL(list);

	common_iter_op(iter);
	entries = &list->_entries;

	if (!IS_LIST_EMPTY(entries)) {
		node = (mcl_list_node *)iter->ptr;	
		ent = &node->_entry;
		if (ent->_next == entries) {
			iter->ptr = NULL;
		}
		else {
			iter->_ptr = (void *)MCL_LIST_ENTRY(ent->_prev);
		}
	}
	else {
		iter->_ptr = NULL;
	}

	return iter->_ptr;
}

static mcl_list_node *mcl_def_list_iter_info(mcl_iter *iter, mcl_list *list)
{
	POINTER_RET_NULL(iter);
	POINTER_RET_NULL(list);

	// TODO
}

mcl_list *mcl_list_new()
{
	mcl_list *listp = (mcl_list *)malloc(sizeof(*listp));
	listp->_list_sz = 0;
	listp->entries = INIT_LIST(listp->_entries);

	INIT_DEFAULT_FUNC(listp);

	return listp;
}

void mcl_list_delete(mcl_list *list_ptr)
{
	mcl_list_head *ent = NULL;
	mcl_list_head *entries = &list_ptr->_entries;
	mcl_list_node *node = NULL;
	if (!IS_LIST_EMPTY(entries)) {
		for (ent = entries->_next; ent != entries;) {
			node = MCL_LIST_ENTRY(ent);
			if (node->_data) {
				free(node->_data);
			}
			ent = ent->next;
			free(node);
		}
	}

	free(list_ptr);
}

int mcl_list_insert(void *data, mcl_list *list_ptr)
{
	if (list_ptr->_list_sz >= MCL_LIST_MAX_SIZE) {
		printf(stderr, "list [%p] size beyond max size [%d]", list_ptr, MCL_LIST_MAX_SIZE);
		return 0;
	}

	mcl_list_node *node = (mcl_list_node *)malloc(sizeof(*node));

	node->_data = data;
	LIST_INSERT(&node->_entry, &list_ptr->_entries);
	++ list_ptr->_list_sz;

	return 1;
}

int mcl_list_delete(void *data, mcl_list *list_ptr)
{
}

