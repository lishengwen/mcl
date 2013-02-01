#ifndef __MCL_ITERATOR_H__
#define __MCL_ITERATOR_H__

typedef struct mcl_iter_s {
	void *_ptr; // �����ڵ�ָ��
	void *_data; // �û�����
	int _index; // �������������е��±�
	int _sz;
} mcl_iter;

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
