#ifndef __MCL_ITERATOR_H__
#define __MCL_ITERATOR_H__

typedef struct mcl_iter_s {
	void *_ptr; // 容器节点指针
	void *_data; // 用户数据
	int _index; // 迭代器在容器中的下标
	int _sz;
} mcl_iter;

#define MCL_ITER_INIT(_iter) \
	do { \
		_iter->_ptr = NULL; \
		_iter->_data = NULL; \
		_iter->_index = -1; \
		_iter->_sz = 0; \
	} while(0)

#define MCL_FOREACH(_iter, _container_ptr) \
	for ((_container_ptr)->iter_head(&(_iter), (_container_ptr)); \
			(_iter).ptr; \
			(_container_ptr)->iter_next(&(_iter), (_container_ptr)))

#define MCL_FOREACH_REVERSE(_iter, _container_ptr) \
	for ((_container_ptr)->iter_tail(&(_iter), (_container_ptr)); \
			(_iter).ptr; \
			(_container_ptr)->iter_prev(&(_iter), (_container_ptr)))

#define MCL_ITERATOR_INFO(_iter, _container_ptr) \
	(_container_ptr)->iter_info(&(_iter), (_container_ptr))

#endif
