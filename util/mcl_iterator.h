#ifndef __MCL_ITERATOR_H__
#define __MCL_ITERATOR_H__

typedef struct mcl_iter_s {
	void *_ptr; // �����ڵ�ָ��
	void *_data; // �û�����
	int _sz;
	union __u {
		int _index; // �������������е��±�
		struct __map_pair {
			void *_key;
			void *_val;
		} _pair; // ��������ȡ��������ֵ��Ϣ
	} _u;
	int _cursor; // �����α�
} mcl_iter;

#define MCL_ITER_INIT(_iter) \
	do { \
		(_iter)->_ptr = NULL; \
		(_iter)->_data = NULL; \
		(_iter)->_sz = 0; \
		(_iter)->_cursor = -1; \
	} while(0)

#define MCL_ITER_INITIALIZER \
	{NULL, NULL, -1, 0, -1}

#define MCL_FOREACH(_iter, _container_ptr) \
	for ((_container_ptr)->iter_head(&(_iter), (_container_ptr)); \
			(_iter)._ptr; \
			(_container_ptr)->iter_next(&(_iter), (_container_ptr)))

#define MCL_FOREACH_REVERSE(_iter, _container_ptr) \
	for ((_container_ptr)->iter_tail(&(_iter), (_container_ptr)); \
			(_iter)._ptr; \
			(_container_ptr)->iter_prev(&(_iter), (_container_ptr)))

#define MCL_ITERATOR_INFO(_iter, _container_ptr) \
	(_container_ptr)->iter_info(&(_iter), (_container_ptr))

#define MCL_ITERATOR_ERASE(_iter, _container_ptr) \
	(_container_ptr)->iter_erase(&(_iter), (_container_ptr))

#define ITER_NULL(_iterp) \
	((_iterp)->_ptr == NULL)

#endif
