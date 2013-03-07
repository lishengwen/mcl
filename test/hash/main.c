#include <util/mcl_hash.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static const char *all_str_keys[] = {"hello", "world", ",", "elliot", "fucks", "it", "kiss", "my", "ass", "bitches", NULL};

typedef struct node_type_s {
	int i;
	int *j;
} node_type;

void print_node(node_type *node)
{
	printf("node addr[%p], i[%d], j_addr[%p], j[%d]\n", 
			node, node->i, node->j, *(node->j));
}

void free_node_fn(void *data)
{
	node_type *node = (node_type *)data;
	if (node) {
		free(node->j);
		free(node);
		printf("free node %p\n", node);
	}
}

node_type *make_node()
{
	node_type *data = NULL;

	data = (node_type *)malloc(sizeof(*data));

	data->i = rand() % 10000;
	data->j = (int *)malloc(sizeof(int));
	*(data->j) = data->i;

	return data;
}

void print_int_hash(mcl_hash *ht)
{
	mcl_iter it = MCL_ITER_INITIALIZER;
	node_type *node = NULL;
	int *key = NULL;

	MCL_FOREACH_REVERSE(it, ht) {
		key = (int *)mcl_hash_iter_key(&it);
		node = (node_type *)mcl_hash_iter_val(&it);
		printf("key=%d, node=%p, content:\n\t", *key, node);
		print_node(node);
	}
}

void print_str_hash(mcl_hash *ht)
{
	mcl_iter it = MCL_ITER_INITIALIZER;
	node_type *node = NULL;
	mcl_string *key = NULL;

	MCL_FOREACH_REVERSE(it, ht) {
		key = (mcl_string *)mcl_hash_iter_key(&it);
		node = (node_type *)mcl_hash_iter_val(&it);
		printf("key=\"%s\", node=%p, content:\n\t", MCL_STR_PTR(key), node);
		print_node(node);
	}
}

int main(int argc, char *argv[])
{
	mcl_hash *int_ht = mcl_int_hash_new();
	mcl_hash *str_ht = mcl_str_hash_new();
	int i = 0;
	int int_key = 0;
	mcl_string *str_key = NULL;
	node_type *data = NULL;
	const char *strp = NULL;

	srand(time(NULL));

	for (i = 0; i < 20; ++ i) {
		data = make_node();
		//int_key = data->i;
		int_key = i;
		mcl_int_hash_insert(int_key, data, int_ht, free_node_fn);
	}

	data = (node_type *)mcl_int_hash_find(18, int_ht);
	if (data) {
		printf("found for key=%d\n", int_key);
		print_node(data);
	}

	mcl_int_hash_insert(18, make_node(), int_ht, free_node_fn);
	mcl_int_hash_insert(100, make_node(), int_ht, free_node_fn);

	print_int_hash(int_ht);


	for (i = 0; i < sizeof(all_str_keys); ++ i) {
		data = make_node();
		strp = all_str_keys[i];
		if (strp == NULL) break;
		str_key = mcl_string_alloc(strp, strlen(strp));
		mcl_str_hash_insert(str_key, data, str_ht, free_node_fn);
	}

	strp = all_str_keys[0];
	str_key = mcl_string_alloc(strp, strlen(strp));
	data = (node_type *)mcl_str_hash_find(str_key, str_ht);
	if (data) {
		printf("found for key=%s\n", MCL_STR_PTR(str_key));
		print_node(data);
	}

	mcl_str_hash_delete(str_key, str_ht, free_node_fn);

	print_str_hash(str_ht);

	printf("over, free htables\n");
	mcl_hash_destroy(int_ht, free_node_fn);
	mcl_hash_destroy(str_ht, free_node_fn);

	return 1;
}

