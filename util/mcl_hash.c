#include <mcl_hash.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

static inline mcl_hash_pair *make_pair(void *key, mcl_data_type type, void *val);
static inline void destroy_pair(mcl_hash_pair *pair, mcl_free_fn_t free_fn);

static void unset_iter(mcl_iter *it)
{
	it->_ptr = NULL;
	it->_data = NULL;
	it->_u._pair._key = NULL;
	it->_u._pair._val = NULL;
	it->_cursor = -1;
}

static void *def_hash_iter_head(mcl_iter *it, mcl_hash *ht)
{
	mcl_list **tables = ht->_tables;
	mcl_list *list = NULL;
	mcl_hash_pair *pair = NULL;
	int max_size = ht->_max_hash_size;
	int i = 0;

	MCL_IF_NOT_RET(it, NULL);
	MCL_IF_NOT_RET(ht, NULL);

	for (i = 0; i < max_size; ++ i) {
		// find a not empty list
		list = tables[i];
		if (list && mcl_list_size(list) > 0) {
			break;
		}
	}

	if (!list) {
		unset_iter(it);
	}
	else {
		list->iter_head(it, list);
		pair = (mcl_hash_pair *)list->iter_info(it, list);
		if (pair) {
			it->_u._pair._key = pair->_key->_key_data;
			it->_u._pair._val = pair->_val;
			it->_cursor = i;
		}
		else {
			unset_iter(it);
		}
	}

	return pair;
}

static void *def_hash_iter_tail(mcl_iter *it, mcl_hash *ht)
{
	// NO support for reverse foreach
	return def_hash_iter_head(it, ht);
}

static void *def_hash_iter_next(mcl_iter *it, mcl_hash *ht)
{
	int cursor = it->_cursor;
	mcl_list **tables = ht->_tables;
	mcl_list *list = NULL;
	mcl_hash_pair *pair = NULL;
	int i = 0;
	int max_size = ht->_max_hash_size;

	MCL_IF_NOT_RET(it, NULL);
	MCL_IF_NOT_RET(ht, NULL);
	MCL_IF_NOT_RET(cursor >= 0 && cursor < max_size, (unset_iter(it), NULL));

	list = tables[cursor];
	if (!list) {
		unset_iter(it);
		fprintf(stderr, "NULL list when traversal htable %p\n", ht);
		return NULL;
	}

	list->iter_next(it, list);
	if (!ITER_NULL(it)) {
		pair = (mcl_hash_pair *)list->iter_info(it, list);
		if (pair) {
			it->_u._pair._key = pair->_key->_key_data;
			it->_u._pair._val = pair->_val;
		}
		else {
			// error! 
			// TODO log
			unset_iter(it);
		}
	}
	else {
		// find next list
		unset_iter(it);
		list = NULL;
		for (i = cursor + 1; i < max_size; ++ i) {
			list = tables[i];
			if (list && mcl_list_size(list) > 0) {
				break;
			}
		}

		if (list) {
			list->iter_head(it, list);
			pair = (mcl_hash_pair *)list->iter_info(it, list);
			if (pair) {
				it->_u._pair._key = pair->_key->_key_data;
				it->_u._pair._val = pair->_val;
				it->_cursor = i;
			}
			else {
				unset_iter(it);
			}
		}
	}

	return pair;
}

static void *def_hash_iter_prev(mcl_iter *it, mcl_hash *ht)
{
	// NO support for reverse foreach
	def_hash_iter_next(it, ht);
}

static void *def_hash_iter_info(mcl_iter *it, mcl_hash *ht)
{
	// return user data
	MCL_IF_NOT_RET(it, NULL);
	MCL_IF_NOT_RET(ht, NULL);

	return it->_u._pair._val;
}

static mcl_iter *def_hash_iter_erase(mcl_iter *it, mcl_hash *ht)
{
	int cursor = it->_cursor;
	int max_size = ht->_max_hash_size;
	mcl_hash_pair *pair = NULL;
	mcl_list **tables = ht->_tables;

	MCL_IF_NOT_RET(it, NULL);
	MCL_IF_NOT_RET(ht, NULL);
	MCL_IF_NOT_RET(cursor >= 0 && cursor < max_size, (unset_iter(it), NULL));
	
	mcl_list *list = tables[cursor];
	if (!list) {
		unset_iter(it);
		return it;
	}

	pair = (mcl_hash_pair *)list->iter_info(it, list);

	def_hash_iter_next(it, ht);	

	if (pair) {
		mcl_list_delete(pair, list, NULL);

		if (ht->erase_fn) {
			destroy_pair(pair, ht->erase_fn);
		}
		else {
			destroy_pair(pair, NULL);
		}
	}

	return it;
}

static inline void init_iter_func(mcl_hash *ht)
{
	ht->iter_head = def_hash_iter_head;
	ht->iter_tail = def_hash_iter_tail;
	ht->iter_next = def_hash_iter_next;
	ht->iter_prev = def_hash_iter_prev;
	ht->iter_info = def_hash_iter_info;
	ht->iter_erase = def_hash_iter_erase;
}

static inline unsigned int cal_hash_int(int key)
{
	return (key * 2654435761) % MCL_HASH_MAX_SIZE;
}

static inline unsigned int cal_hash_str(mcl_string *strp)
{
	const char *str = (const char *)MCL_STR_PTR(strp);
	int len = MCL_STR_LEN(strp);
	// ELFHASH
	long hash = 0;
	long x = 0;
	int i = 0;
	for(; i < len; ++ i) {
		hash = (hash << 4) + (long)str[i];
		if ((x = hash & 0xF0000000) != 0) {
			hash ^= (x >> 24);
		}
		hash &= ~x;
	}

	return cal_hash_int(hash);
}

static inline int available_hash_type(mcl_data_type type)
{
	if (type != MCL_T_INT && type != MCL_T_STR) {
		return 0;
	}

	return 1;
}

static int calc_hash(void * key, mcl_data_type type)
{
	if (type == MCL_T_INT) {
		return cal_hash_int(*(int * )key);
	}
	else if (type == MCL_T_STR) {
		return cal_hash_str((mcl_string *)key);
	}
	else {
		return -1;
	}
}

static inline int available_hash_value(int hash)
{
	return hash >= 0 && hash < MCL_HASH_MAX_SIZE;
}

static inline void free_hash_key(mcl_hash_key *key)
{
	if (!key) return;

	if (key->_key_data) {
		if (key->_type == MCL_T_INT) {
			free(key->_key_data);
		}
		else if (key->_type == MCL_T_STR) {
			mcl_string_free(key->_key_data);
		}
		else {
			// error type
			fprintf(stderr, "wrong hash key type when free %p, type=%d", key, key->_type);
		}
	}
	free(key);
}

static int key_cmp(mcl_hash_key *key1, mcl_hash_key *key2)
{
	if (key1->_type != key2->_type) return 0;

	if (key1->_type == MCL_T_INT) {
		/*
		printf("compare key=%d with key=%d\n",
				*(int *)(key1->_key_data), *(int *)(key2->_key_data));
		*/
		return *(int *)(key1->_key_data) == *(int *)(key2->_key_data);
	}
	else if (key1->_type == MCL_T_STR) {
		return mcl_string_cmp(key1->_key_data, key2->_key_data) == 0;
	}
	else {
		return 0;
	}
}

static int pair_pair_cmp(void *pair_data, void *dst_data)
{
	mcl_hash_pair *pair1 = (mcl_hash_pair *)pair_data;
	mcl_hash_pair *pair2 = (mcl_hash_pair *)dst_data;
	return key_cmp(pair1->_key, pair2->_key);
}

static int key_pair_cmp(void *key_data, void *pair_data)
{
	mcl_hash_key *key = (mcl_hash_key *)key_data;
	mcl_hash_pair *pair = (mcl_hash_pair *)pair_data;

	MCL_IF_NOT_RET(key, 0);
	MCL_IF_NOT_RET(pair, 0);

	mcl_hash_key *dst_key = pair->_key;

	//printf("compare key[%p] with key[%p]\n", key, dst_key);

	return key_cmp(key, dst_key);
}

static inline mcl_hash_pair *make_pair(void *key, mcl_data_type type, void *val)
{
	mcl_hash_pair *pair = NULL;

	pair = (mcl_hash_pair *)malloc(sizeof(*pair));
	pair->_key = (mcl_hash_key *)malloc(sizeof(mcl_hash_key));
	pair->_key->_type = type;
	if (type == MCL_T_STR) {
		pair->_key->_key_data = mcl_string_copy((mcl_string*)key);
		//printf("make pair, key=%s\n", MCL_STR_PTR(((mcl_string*)key)));
	}
	else if (type == MCL_T_INT) {
		pair->_key->_key_data = (int *)malloc(sizeof(int));
		*(int *)(pair->_key->_key_data) = *(int *)key;
		//printf("make pair, key=%d\n", *(int *)key);
	}
	else {
		return NULL;
	}

	pair->_val = val;

	return pair;
}

static inline void destroy_pair(mcl_hash_pair *pair, mcl_free_fn_t free_fn)
{
	mcl_hash_key *key = NULL;
	void *data = NULL;

	if (!pair) return;

	key = pair->_key;
	if (key) {
		free_hash_key(key);
	}
	data = pair->_val;
	if (data && free_fn) {
		free_fn(data);
	}

	free(pair);
}

static inline void init_hash_tables(mcl_hash *ht)
{
	int i = 0;
	if (!ht) return;

	if (!ht->_tables) return;

	for (i = 0; i < ht->_max_hash_size; ++ i) {
		ht->_tables[i] = NULL;
	}
}

static inline mcl_hash *mcl_hash_new(mcl_data_type type)
{
	mcl_hash *new_hash = NULL;

	if (!available_hash_type(type)) {
		return NULL;
	}

	new_hash = (mcl_hash *)malloc(sizeof(*new_hash));
	new_hash->_tables = (mcl_list **)malloc(sizeof(mcl_list *) * MCL_HASH_MAX_SIZE);

	new_hash->_max_hash_size = MCL_HASH_MAX_SIZE;
	new_hash->_cur_elem_num = 0;
	new_hash->_key_type = type;
	new_hash->erase_fn = NULL;

	init_hash_tables(new_hash);

	init_iter_func(new_hash);

	return new_hash;
}

mcl_hash *mcl_int_hash_new()
{
	return mcl_hash_new(MCL_T_INT);
}

mcl_hash *mcl_str_hash_new()
{
	return mcl_hash_new(MCL_T_STR);
}

void mcl_hash_destroy(mcl_hash *ht, mcl_free_fn_t free_fn)
{
	if (!ht) return;

	mcl_hash_clean(ht, free_fn);

	free(ht->_tables);

	free(ht);
}

void mcl_hash_clean(mcl_hash *ht, mcl_free_fn_t free_fn)
{
	int i = 0;
	mcl_list ** tables = ht->_tables;
	mcl_list *list = NULL;
	mcl_hash_pair *pair = NULL;
	mcl_hash_key *key = NULL;
	void *data = NULL;

	if (!ht) return;

	mcl_iter list_it = MCL_ITER_INITIALIZER;

	for (i = 0; i < ht->_max_hash_size; ++ i) {
		list = tables[i];
		if (list == NULL) continue; // mostly
		MCL_FOREACH(list_it, list) {
			pair = (mcl_hash_pair *)MCL_ITERATOR_INFO(list_it, list);
			if (pair) {
				destroy_pair(pair, free_fn);
			}
		}
		mcl_list_destroy(list, NULL);
		tables[i] = NULL;
	}
}

static int do_hash_insert(int hash, mcl_hash_pair *pair, mcl_hash *ht, int sub_flg, mcl_free_fn_t free_fn)
{
	mcl_list *list = NULL;
	mcl_hash_pair *pair_found = NULL;

	if (!available_hash_value(hash)) return 0;

	// find index
	if (!ht->_tables[hash]) {
		ht->_tables[hash] = mcl_list_new();
	}
	list = ht->_tables[hash];

	pair_found = (mcl_hash_pair*)mcl_list_find(pair, list, pair_pair_cmp);	
	if (pair_found) {
		if (!sub_flg) {
			return 0;
		}
		else {
			mcl_list_delete(pair_found, list, NULL);
			destroy_pair(pair_found, free_fn);
		}
	}

	return mcl_list_insert(pair, list);
}

static inline int hash_insert(void * key, mcl_data_type key_type, void *data, mcl_hash *ht, int sub_flg, mcl_free_fn_t free_fn)
{
	mcl_hash_pair *pair = NULL;
	int hash = 0;
	int ret = 0;

	MCL_IF_NOT_RET(ht, 0);
	MCL_IF_NOT_RET(key, 0);
	MCL_IF_NOT_RET(data, 0);
	assert(ht->_key_type == key_type);
	
	hash = calc_hash(key, key_type);
	pair = make_pair(key, key_type, data);

	ret = do_hash_insert(hash, pair, ht, sub_flg, free_fn);
	if (!ret) destroy_pair(pair, NULL);
	/*
	else {
		printf("insert ok, key type=%d, hash=%d, data=%p\n", 
				key_type, hash, data);
	}
	*/

	return ret;
}

int mcl_int_hash_insert_safe(int key, void *data, mcl_hash *ht)
{
	return hash_insert(&key, MCL_T_INT, data, ht, 0, NULL);
}

int mcl_str_hash_insert_safe(mcl_string *key, void *data, mcl_hash *ht)
{
	return hash_insert(key, MCL_T_STR, data, ht, 0, NULL);
}

int mcl_int_hash_insert(int key, void *data, mcl_hash *ht, mcl_free_fn_t free_fn)
{
	return hash_insert(&key, MCL_T_INT, data, ht, 1, free_fn);
}

int mcl_str_hash_insert(mcl_string *key, void *data, mcl_hash *ht, mcl_free_fn_t free_fn)
{
	return hash_insert(key, MCL_T_STR, data, ht, 1, free_fn);
}

static int hash_delete(int hash, mcl_hash_key *ht_key, mcl_hash *ht, mcl_free_fn_t free_fn)
{
	mcl_list **tables = ht->_tables;
	mcl_list *list = NULL;
	mcl_hash_pair *pair_found = NULL;

	if (!available_hash_value(hash)) return 0;

	list = tables[hash];
	if (!list) {
		return 0;
	}

	pair_found = (mcl_hash_pair *)mcl_list_find(ht_key, list, key_pair_cmp);
	if (pair_found) {
		mcl_list_delete(pair_found, list, NULL);
		destroy_pair(pair_found, free_fn);
		return 1;
	}

	return 0;
}

int mcl_int_hash_delete(int key, mcl_hash *ht, mcl_free_fn_t free_fn)
{
	int hash = 0;

	MCL_IF_NOT_RET(ht, -1);
	assert(ht->_key_type == MCL_T_INT);
	
	hash = cal_hash_int(key);
	mcl_hash_key ht_key;
	ht_key._type = MCL_T_INT;
	ht_key._key_data = &key;

	return hash_delete(hash, &ht_key, ht, free_fn);
}

int mcl_str_hash_delete(mcl_string *key, mcl_hash *ht, mcl_free_fn_t free_fn)
{
	int hash = 0;

	MCL_IF_NOT_RET(ht, -1);
	MCL_IF_NOT_RET(key, -1);
	assert(ht->_key_type == MCL_T_STR);
	
	hash = cal_hash_str(key);
	mcl_hash_key ht_key;
	ht_key._type = MCL_T_STR;
	ht_key._key_data = key;

	return hash_delete(hash, &ht_key, ht, free_fn);
}

static void *hash_find(int hash, mcl_hash_key *ht_key, mcl_hash *ht)
{
	mcl_list **tables = ht->_tables;
	mcl_list *list = NULL;
	mcl_hash_pair *pair_found = NULL;

	if (!available_hash_value(hash)) return NULL;

	list = tables[hash];
	if (!list) {
		return NULL;
	}

	//printf("find list=%p\n", list);

	pair_found = (mcl_hash_pair *)mcl_list_find(ht_key, list, key_pair_cmp);
	if (pair_found) {
		return pair_found->_val;
	}

	return NULL;
}

void *mcl_int_hash_find(int key, mcl_hash *ht)
{
	int hash = 0;

	MCL_IF_NOT_RET(ht, NULL);
	assert(ht->_key_type == MCL_T_INT);
	
	hash = cal_hash_int(key);
	mcl_hash_key ht_key;
	ht_key._type = MCL_T_INT;
	ht_key._key_data = &key;

	//printf("hash find, prepare hash=%d, key=%d\n", hash, key);

	return hash_find(hash, &ht_key, ht);
}

void *mcl_str_hash_find(mcl_string *key, mcl_hash *ht)
{
	int hash = 0;

	MCL_IF_NOT_RET(ht, NULL);
	MCL_IF_NOT_RET(key, NULL);
	assert(ht->_key_type == MCL_T_STR);
	
	hash = cal_hash_str(key);
	mcl_hash_key ht_key;
	ht_key._type = MCL_T_STR;
	ht_key._key_data = key;

	return hash_find(hash, &ht_key, ht);
}

int mcl_hash_ctrl(mcl_ds_ctrl_cmd cmd, void *data, mcl_hash *ht)
{
	mcl_free_fn_t free_fn = NULL;

	MCL_IF_NOT_RET(data, 0);
	MCL_IF_NOT_RET(ht, 0);

	switch (cmd) {
		case MCL_REGIST_ERASE_FN:
			free_fn = (mcl_free_fn_t) data;
			ht->erase_fn = free_fn;
			return 1;
		case MCL_UNREGIST_ERASE_FN:
			ht->erase_fn = NULL;
			return 1;
		default:
			break;
	}

	return 0;
}

void *mcl_hash_iter_key(const mcl_iter *iter)
{
	MCL_IF_NOT_RET(iter, NULL);

	return iter->_u._pair._key;
}

void *mcl_hash_iter_val(const mcl_iter *iter)
{
	MCL_IF_NOT_RET(iter, NULL);

	return iter->_u._pair._val;
}

