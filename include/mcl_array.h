#ifndef __MCL_ARRAY_H__
#define __MCL_ARRAY_H__

#include <mcl_common.h>
#include <mcl_iterator.h>

MCL_HEADER_BEGIN

#define MCL_ARR_DEFAULT_SIZE (64)

typedef struct mcl_array_s {
	void **_all_data;
	unsigned int _cur_size;
	unsigned int _total_size;
	// TODO support mem pool
	
	void *(*iter_head)(mcl_iter *iter, struct mcl_array_s *container);
	void *(*iter_tail)(mcl_iter *iter, struct mcl_array_s *container);
	void *(*iter_next)(mcl_iter *iter, struct mcl_array_s *container);
	void *(*iter_prev)(mcl_iter *iter, struct mcl_array_s *container);
	void *(*iter_info)(mcl_iter *iter, struct mcl_array_s *containter);
	mcl_iter *(*iter_erase)(mcl_iter *iter, struct mcl_array_s *container);
} mcl_array;

mcl_array *mcl_array_new(int init_size);
// free array and all elements it holds by function of free_fn if exists
void mcl_array_destroy(mcl_array *array, mcl_free_fn_t free_fn);
// clean all elements and destroy them if free_fn exists, but keep array itself
void mcl_array_clean(mcl_array *array, mcl_free_fn_t free_fn);
// tighten memory consumed by array
void mcl_array_tighten(mcl_array *array);

// insert data at the end of array
int mcl_array_append(void *data, mcl_array *array);

// insert data at the beginning of array
int mcl_array_preappend(void *data, mcl_array *array);

// insert data after the specific position
int mcl_array_insert(void *data, int index, mcl_array *array);

// insert data before the specific position
int mcl_array_preinsert(void *data, int index, mcl_array *array);

// delete specific position elem, and keep original sequence
int mcl_array_delete_indx(int index, mcl_array *array, mcl_free_fn_t free_fn);
// delete specific position elem, and move last elem to this position
int mcl_array_delete(int index, mcl_array *array, mcl_free_fn_t free_fn);
// delete specific position elem, and move last elem to this position
int mcl_array_delete_obj(void *data, mcl_array *array, mcl_free_fn_t free_fn);
// delete elem from bindex to eindex, which means range of [bindex, eindex), and keep original sequence
int mcl_array_delete_range(int bindx, int eindx, mcl_array *array, mcl_free_fn_t free_fn);

// get data at the specific index
void *mcl_array_at(int index, mcl_array *array);

int mcl_array_size(mcl_array *array);
int mcl_array_capacity(mcl_array *array);

#define MCL_SORT_ASC (1)
#define MCL_SORT_DES (-1)
// quick sort array by order, which is ascending when order > 1, otherwise descending
void mcl_array_qsort(int order, mcl_array *array, int (*cmp_fn)(void *one, void *another));

MCL_HEADER_END

#endif
