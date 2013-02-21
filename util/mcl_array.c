#include <mcl_array.h>

#define POINTER_NULL_RET(_ptr) \
	do { \
		if (_ptr == NULL) return; \
	} while (0)

#define INCREASE_SIZE(_arr) \
	do { \
		++ _arr->_cur_size; \
	} while (0)

#define DECREASE_SIZE(_arr) \
	do { \
		-- _arr->_cur_size; \
		if (_arr->_cur_size < 0) { \
			_arr->_cur_size = 0; \
		} \
	} while (0)

static inline void unset_iter(mcl_iter *iter)
{
	iter->_ptr = NULL;
	iter->_data = NULL;
	iter->_u._index = -1;
}

static inline int check_out_of_bounds(int index, mcl_array *array)
{
	if (!array) {
		return 1;
	}
	if (array->_cur_size == 0) {
		return index < 0 || index > array->_cur_size;
	}
	else {
		return index < 0 || index >= array->_cur_size;
	}
}

static void *def_iter_head(mcl_iter *iter, mcl_array *array)
{
	MCL_IF_NOT_RET(iter, NULL);
	MCL_IF_NOT_RET(array, NULL);

	if (array->_cur_size <= 0) {
		unset_iter(iter);
	}
	else {
		iter->_ptr = iter->_data = array->_all_data[0];
		iter->_u._index = 0;
	}
	return iter->_ptr;
}

static void *def_iter_tail(mcl_iter *iter, mcl_array *array)
{
	MCL_IF_NOT_RET(iter, NULL);
	MCL_IF_NOT_RET(array, NULL);

	if (array->_cur_size <= 0) {
		unset_iter(iter);
	}
	else {
		iter->_ptr = iter->_data = array->_all_data[array->_cur_size - 1];
		iter->_u._index = array->_cur_size - 1;
	}

	return iter->_ptr;
}

static void *def_iter_next(mcl_iter *iter, mcl_array *array)
{
	MCL_IF_NOT_RET(iter, NULL);
	MCL_IF_NOT_RET(array, NULL);

	++ iter->_u._index;
	if (check_out_of_bounds(iter->_u._index, array)) {
		unset_iter(iter);
	}
	else {
		iter->_ptr = iter->_data = array->_all_data[iter->_u._index];
	}

	return iter->_ptr;
}

static void *def_iter_prev(mcl_iter *iter, mcl_array *array)
{
	MCL_IF_NOT_RET(iter, NULL);
	MCL_IF_NOT_RET(array, NULL);

	-- iter->_u._index;
	if (check_out_of_bounds(iter->_u._index, array)) {
		unset_iter(iter);
	}
	else {
		iter->_ptr = iter->_data = array->_all_data[iter->_u._index];
	}

	return iter->_ptr;
}

static void *def_iter_info(mcl_iter *iter, mcl_array *array)
{
	MCL_IF_NOT_RET(iter, NULL);
	MCL_IF_NOT_RET(array, NULL);

	if (check_out_of_bounds(iter->_u._index, array)) {
		unset_iter(iter);
		return NULL;
	}
	else {
		return iter->_data;
	}
}

static mcl_iter *def_iter_erase(mcl_iter *iter, mcl_array *array)
{
	int index = iter->_u._index;

	MCL_IF_NOT_RET(iter, NULL);
	MCL_IF_NOT_RET(array, NULL);

	if (check_out_of_bounds(index, array)) {
		unset_iter(iter);
		return NULL;
	}
	else {
		if (array->erase_fn) {
			index = mcl_array_delete_indx(index, array, array->erase_fn);
		}
		else {
			index = mcl_array_delete_indx(index, array, NULL);
		}
		if (index < 0) {
			unset_iter(iter);
			return NULL;
		}
		else {
			iter->_u._index = index;
			iter->_ptr = iter->_data = array->_all_data[index];
			return iter;
		}
	}
}

static inline void set_def_iter_func(mcl_array *array)
{
	array->iter_head = def_iter_head;
	array->iter_tail = def_iter_tail;
	array->iter_next = def_iter_next;
	array->iter_prev = def_iter_prev;
	array->iter_info = def_iter_info;
	array->iter_erase = def_iter_erase;
}

mcl_array *mcl_array_new(int init_size)
{
	int i = 0;
	int capacity = 0;
	if (init_size < 0) {
		return NULL;
	}
	mcl_array *ret_arr = (mcl_array *)malloc(sizeof(*ret_arr));

	if (init_size > MCL_ARR_DEFAULT_SIZE) {
		capacity = init_size << 1;
	}
	else {
		capacity = MCL_ARR_DEFAULT_SIZE;
	}

	ret_arr->_all_data = (void **)malloc(sizeof(void *) * capacity);
	for (i = 0; i < capacity; ++ i) {
		ret_arr->_all_data[i] = NULL;
	}

	ret_arr->_cur_size = 0;
	ret_arr->_total_size = capacity;

	set_def_iter_func(ret_arr);

	return ret_arr;
}

void mcl_array_destroy(mcl_array *array, mcl_free_fn_t free_fn)
{
	POINTER_NULL_RET(array);

	mcl_array_clean(array, free_fn);

	if (array->_all_data) {
		free(array->_all_data);
	}

	free(array);
}

void mcl_array_clean(mcl_array *array, mcl_free_fn_t free_fn)
{
	POINTER_NULL_RET(array);

	int size = array->_cur_size;
	int i = 0;
	for (; i < size; ++ i) {
		if (free_fn != NULL && array->_all_data[i] != NULL) {
			free_fn(array->_all_data[i]);
		}
		array->_all_data[i] = NULL;
	}

	array->_cur_size = 0;
}

void mcl_array_tighten(mcl_array *array)
{
	POINTER_NULL_RET(array);

	int size = array->_cur_size;
	int capacity = array->_total_size;
	void **orign_ptr = array->_all_data;
	int expect_sz = size << 1;

	if (capacity <= expect_sz) {
		return;
	}

	capacity = expect_sz;

	array->_all_data = (void **)realloc(orign_ptr, sizeof(void *) * capacity);
	array->_total_size = capacity;
}

static inline void extend_array(int new_size, mcl_array *array)
{
	if (new_size <= array->_total_size) {
		return;
	}

	void **orgin_ptr = array->_all_data;
	array->_all_data = (void **)realloc(orgin_ptr, sizeof(void *) * new_size);
	array->_total_size = new_size;
}

static inline void try_extend_array(mcl_array *array)
{
	int cur_size = array->_cur_size;
	int total_size = array->_total_size;
	if (total_size <= cur_size) {
		extend_array(total_size << 1, array);
	}
}

int mcl_array_append(void *data, mcl_array *array)
{
	int cur_size = array->_cur_size;

	try_extend_array(array);

	array->_all_data[cur_size] = data;
	INCREASE_SIZE(array);

	return cur_size;
}

int mcl_array_preappend(void *data, mcl_array *array)
{
	int cur_size = array->_cur_size;
	int i = 0;

	try_extend_array(array);

	for (i = cur_size; i > 0; -- i) {
		array->_all_data[i] = array->_all_data[i - 1];
	}

	array->_all_data[i] = data;

	INCREASE_SIZE(array);

	return 0;
}

int mcl_array_insert(void *data, int index, mcl_array *array)
{
	int i = 0;
	if (check_out_of_bounds(index, array)) {
		return -1;
	}

	if (index == array->_cur_size - 1) {
		return mcl_array_append(data, array);
	}
	else {
		try_extend_array(array);
		for (i = array->_cur_size - 1; i > index; -- i) {
			array->_all_data[i + 1] = array->_all_data[i];
		}
		array->_all_data[++ index] = data;

		INCREASE_SIZE(array);

		return index;
	}
}

int mcl_array_preinsert(void *data, int index, mcl_array *array)
{
	int i = 0;
	if (check_out_of_bounds(index, array)) {
		return -1;
	}

	if (index == 0) {
		return mcl_array_preappend(data, array);
	}
	else {
		try_extend_array(array);
		for (i = array->_cur_size - 1; i >= index; -- i) {
			array->_all_data[i + 1] = array->_all_data[i];
		}
		array->_all_data[index] = data;

		INCREASE_SIZE(array);

		return index;
	}
}

int mcl_array_delete_indx(int index, mcl_array *array, mcl_free_fn_t free_fn)
{
	int i = 0;
	if (check_out_of_bounds(index, array)) {
		return -1;
	}

	if (free_fn != NULL && array->_all_data[index] != NULL) {
		free_fn(array->_all_data[index]);
	}

	for (i = index; i < array->_cur_size - 1; ++ i) {
		array->_all_data[i] = array->_all_data[i + 1];
	}

	array->_all_data[i] = NULL;

	DECREASE_SIZE(array);

	return index;
}

int mcl_array_delete(int index, mcl_array *array, mcl_free_fn_t free_fn)
{
	if (check_out_of_bounds(index, array)) {
		return -1;
	}

	if (free_fn != NULL && array->_all_data[index] != NULL) {
		free_fn(array->_all_data[index]);
	}

	array->_all_data[index] = array->_all_data[array->_cur_size - 1];

	array->_all_data[array->_cur_size - 1] = NULL;

	DECREASE_SIZE(array);

	return index;
}

int mcl_array_delete_obj(void *data, mcl_array *array, mcl_free_fn_t free_fn)
{
	int i = 0;
	for (i = 0; i < array->_cur_size; ++ i) {
		if (data == array->_all_data[i]) {
			break;
		}
	}

	if (i >= array->_cur_size) {
		return -1;
	}

	return mcl_array_delete(i, array, free_fn);
}

int mcl_array_delete_range(int bindex, int eindex, mcl_array *array, mcl_free_fn_t free_fn)
{
	int i = 0;
	int cnt = eindex - bindex;
	if (check_out_of_bounds(bindex, array) ||
			check_out_of_bounds(eindex, array)) {
		return -1;
	}

	if (bindex > eindex) {
		return -1;
	}
	else if (bindex == eindex) {
		mcl_array_delete_indx(bindex, array, free_fn);
		return 1;
	}
	else {
		for (i = bindex; i < eindex; ++ i) {
			if (free_fn != NULL && array->_all_data[i] != NULL) {
				free_fn(array->_all_data[i]);
				array->_all_data[i] = NULL;
			}
		}

		for (; i < array->_cur_size; ++ i) {
			array->_all_data[i - cnt] = array->_all_data[i];
			array->_all_data[i] = NULL;
		}

		array->_cur_size -= cnt;

		return cnt;
	}
}

void *mcl_array_at(int index, mcl_array *array)
{
	if (check_out_of_bounds(index, array)) {
		return NULL;
	}

	return array->_all_data[index];
}

int mcl_array_size(mcl_array *array)
{
	if (!array) return -1;

	return array->_cur_size;
}

int mcl_array_capacity(mcl_array *array)
{
	if (!array) return -1;

	return array->_total_size;
}

int mcl_array_ctrl(mcl_ds_ctrl_cmd cmd, void *data, mcl_array *array)
{
	mcl_free_fn_t free_fn = NULL;

	MCL_IF_NOT_RET(data, 0);
	MCL_IF_NOT_RET(array, 0);

	switch (cmd) {
		case MCL_REGIST_ERASE_FN:
			free_fn = (mcl_free_fn_t) data;
			array->erase_fn = free_fn;
			return 1;
		case MCL_UNREGIST_ERASE_FN:
			array->erase_fn = NULL;
			return 1;
		default:
			break;
	}

	return 0;
}

