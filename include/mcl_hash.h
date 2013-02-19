#ifndef __MCL_HASH_H__
#define __MCL_HASH_H__

#include <mcl_list.h>
#include <mcl_string.h>
#include <mcl_iterator.h>
#include <mcl_array.h>
#include <mcl_common.h>

MCL_HEADER_BEGIN

#define MCL_HASH_MAX_SIZE	(1021) // a prime number

typedef struct mcl_hash_key_s {
	mcl_data_type _type;
	void *_key_data;
} mcl_hash_key;

typedef struct mcl_hash_pair_s {
	mcl_hash_key *_key;
	void *_val;
} mcl_hash_pair;

typedef struct mcl_hash_s {
	mcl_array *_tables;
	int _max_hash_size;
	int _cur_elem_num;
	mcl_data_type _key_type;
	// TODO support mem pool

	void *(*iter_head)(mcl_iter *iter, struct mcl_hash_s *container);
	void *(*iter_tail)(mcl_iter *iter, struct mcl_hash_s *container);
	void *(*iter_next)(mcl_iter *iter, struct mcl_hash_s *container);
	void *(*iter_prev)(mcl_iter *iter, struct mcl_hash_s *container);
	void *(*iter_info)(mcl_iter *iter, struct mcl_hash_s *containter);
	mcl_iter *(*iter_erase)(mcl_iter *iter, struct mcl_hash_s *container);
} mcl_hash;

mcl_hash *mcl_int_hash_new();
mcl_hash *mcl_str_hash_new();

void mcl_hash_destroy(mcl_hash *ht, mcl_free_fn_t free_fn);

int mcl_int_hash_insert(int key, void *data, mcl_hash *ht);
int mcl_str_hash_insert(mcl_string *key, void *data, mcl_hash *ht);

MCL_HEADER_END

#endif

