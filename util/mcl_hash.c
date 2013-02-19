#include <mcl_hash.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define INIT_DEFAULT_FUNC(_hashp) \
	do { \
	} while(0) 

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

static inline void init_hash_tables(mcl_hash *ht)
{
	int i = 0;
	mcl_list *listp = NULL;

	if (!ht) return;

	if (!ht->_tables) return;

	for(i = 0; i < ht->_max_hash_size; ++ i) {
		listp = mcl_list_new();
		mcl_array_append(listp, ht->_tables);
	}
}

/*
static void arr_free_fn(void *data)
{
	mcl_list *listp = (mcl_list *)data;
	mcl_list_destroy(listp, NULL);
}
*/

static inline mcl_hash *mcl_hash_new(mcl_data_type type)
{
	mcl_hash *new_hash = NULL;

	if (!available_hash_type(type)) {
		return NULL;
	}

	new_hash = (mcl_hash *)malloc(sizeof(*new_hash));
	new_hash->_tables = mcl_array_new(MCL_HASH_MAX_SIZE);

	new_hash->_max_hash_size = MCL_HASH_MAX_SIZE;
	new_hash->_cur_elem_num = 0;
	new_hash->_key_type = type;

	init_hash_tables(new_hash);

	INIT_DEFAULT_FUNC(new_hash);

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
	mcl_array *tables = ht->_tables;
	mcl_list *list = NULL;
	mcl_hash_pair *pair = NULL;
	mcl_hash_key *key = NULL;
	void *data = NULL;

	mcl_iter arr_it = MCL_ITER_INITIALIZER;
	mcl_iter lis_it = MCL_ITER_INITIALIZER;

	MCL_FOREACH(arr_it, tables) {
		list = (mcl_list *)MCL_ITERATOR_INFO(arr_it, tables);
		MCL_FOREACH(lis_it, list) {
			pair = (mcl_hash_pair *)MCL_ITERATOR_INFO(lis_it, list);
			if (pair) {
				key = pair->_key;
				if (key) {
					if (key->_key_data) free(key->_key_data);
					free(key);
				}
				data = pair->_val;
				if (data && free_fn) {
					free_fn(data);
				}

				free(pair);
			}
		}
		mcl_list_destroy(list, NULL);
	}

	mcl_array_destroy(tables, NULL);

	free(ht);
}

static int hash_insert(int hash, mcl_hash_pair *pair, mcl_hash *ht)
{
	mcl_list *list = NULL;
	// find index
	/*
	if (hash_ptr->_tables[hash]) {
		hash_ptr->_tables[hash] = mcl_list_new();
	}
	list = hash_ptr->_tables[hash];
	*/
	// TODO
	// wait for mcl_array
}

int mcl_int_hash_insert(int key, void *data, mcl_hash *hash_ptr)
{
	mcl_hash_pair *pair = NULL;
	int hash = 0;
	assert(hash_ptr->_key_type == MCL_T_INT);
	
	hash = cal_hash_int(key);

	pair = (mcl_hash_pair *)malloc(sizeof(*pair));

	pair->_key = (mcl_hash_key *)malloc(sizeof(mcl_hash_key));
	pair->_key->_type = hash_ptr->_key_type;
	pair->_key->_key_data = (int *)malloc(sizeof(int));
	*(int *)pair->_key->_key_data = key;

	return hash_insert(hash, pair, hash_ptr);
}

int mcl_str_hash_insert(mcl_string *key, void *data, mcl_hash *hash_ptr)
{
	mcl_hash_pair *pair = NULL;
	int hash = 0;
	assert(hash_ptr->_key_type == MCL_T_STR);
	
	hash = cal_hash_str(key);

	pair = (mcl_hash_pair *)malloc(sizeof(*pair));

	pair->_key = (mcl_hash_key *)malloc(sizeof(mcl_hash_key));
	pair->_key->_type = hash_ptr->_key_type;
	pair->_key->_key_data = mcl_string_copy(key);

	return hash_insert(hash, pair, hash_ptr);
}

