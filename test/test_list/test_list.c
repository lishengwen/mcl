#include <mcl_list.h>
#include <mcl_iterator.h>
#include <stdio.h>
#include <stdlib.h>

int cmp(int *i, int *j)
{
	if (*i == *j) return 1;

	return 0;
}

int main()
{
	mcl_list * listp = mcl_list_new();

	int *i = (int *)malloc(sizeof(*i));
	int *j = (int *)malloc(sizeof(*j));
	int *k = (int *)malloc(sizeof(*k));
	*i = 1;
	*j = 2;
	*k = 3;
	int *intp = NULL;

	mcl_list_insert_tail(i, listp);
	mcl_list_insert_tail(j, listp);
	mcl_list_insert_tail(k, listp);

	mcl_iter it;
	MCL_ITER_INIT((&it));	
	MCL_FOREACH (it, listp) {
		intp = (int *)MCL_ITERATOR_INFO(it, listp);	
		printf("%p = %d\n", intp, *intp);
	}

	mcl_list_delete(i, listp);

	printf("delete 1...\n");

	MCL_FOREACH (it, listp) {
		intp = (int *)MCL_ITERATOR_INFO(it, listp);	
		printf("%p = %d\n", intp, *intp);
	}

	mcl_list_del_cmp(j, listp, cmp);


	printf("fsdjf\n");

	MCL_FOREACH (it, listp) {
		intp = (int *)MCL_ITERATOR_INFO(it, listp);	
		printf("%p = %d\n", intp, *intp);
	}

	mcl_list_destroy(listp);
	return 1;
}

