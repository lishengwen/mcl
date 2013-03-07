#include <util/mcl_list.h>
#include <util/mcl_iterator.h>
#include <stdio.h>
#include <stdlib.h>

int cmp(void *i, void *j)
{
	int *ip = (int *)i;
	int *jp = (int *)j;
	if (*ip == *jp) return 1;

	return 0;
}

static void print_list(mcl_list *listp)
{
	int *intp;
	mcl_iter it = MCL_ITER_INITIALIZER;
	MCL_FOREACH (it, listp) {
		intp = (int *)MCL_ITERATOR_INFO(it, listp);	
		printf("%p = %d\n", intp, *intp);
	}
}

int main1()
{
	mcl_list * listp = mcl_list_new();
	mcl_list_ctrl(MCL_REGIST_ERASE_FN, free, listp);

	mcl_iter it = MCL_ITER_INITIALIZER;

	int *i = (int *)malloc(sizeof(*i));
	int *j = (int *)malloc(sizeof(*j));
	int *k = (int *)malloc(sizeof(*k));
	int *l = (int *)malloc(sizeof(*l));
	int *intp;
	
	*i = 1;
	*j = 2;
	*k = 3;
	*l = 4;

	mcl_list_insert(i, listp);
	mcl_list_insert_tail(j, listp);
	mcl_list_insert(k, listp);
	mcl_list_insert_tail(l, listp);
	print_list(listp);

	for (listp->iter_head(&it, listp); !ITER_NULL(&it);) {
		intp = (int *)MCL_ITERATOR_INFO(it, listp);	
		printf("%p = %d\n", intp, *intp);
		if (*intp == 4) {
			if (MCL_ITERATOR_ERASE(it, listp)) {
				printf("erase %d\n", *intp);
				continue;
			}
		}

		listp->iter_next(&it, listp);
	}

	printf("after print: \n");

	print_list(listp);

	mcl_list_destroy(listp, MCL_DEF_FREE_FN);
}

int main()
{
	mcl_list * listp = mcl_list_new();

	int *i = (int *)malloc(sizeof(*i));
	int *j = (int *)malloc(sizeof(*j));
	int *k = (int *)malloc(sizeof(*k));
	int *l = (int *)malloc(sizeof(*l));
	
	*i = 1;
	*j = 2;
	*k = 3;
	*l = 4;
	int *intp = NULL;
	int cmpnum = 3;
	int ret = 0;
	int cmpnum2 = 1;

	mcl_list_insert(i, listp);
	mcl_list_insert_tail(j, listp);
	mcl_list_insert(k, listp);
	mcl_list_insert_tail(l, listp);
	print_list(listp);
	printf("\n");

	ret = mcl_list_delete((void *)&cmpnum2, listp, MCL_DEF_FREE_FN);
	printf("delete 2 fake, ret=%d\n", ret);
	print_list(listp);
	printf("\n");

	ret = mcl_list_del_cmp((void *)j, listp, cmp, MCL_DEF_FREE_FN);
	printf("delete 2, ret=%d\n", ret);
	print_list(listp);
	printf("\n");

	//ret = mcl_list_rm_cmp((void *)&cmpnum, listp, cmp);
	ret = mcl_list_del_cmp((void *)&cmpnum, listp, cmp, NULL);
	free(k);
	printf("delete 3, ret=%d\n", ret);
	print_list(listp);
	printf("\n");
	/*
	ret = mcl_list_delete((void *)k, listp);
	printf("delete 3, ret=%d\n", ret);
	print_list(listp);
	printf("\n");
	*/

	//ret = mcl_list_remove(i, listp);
	ret = mcl_list_delete(i, listp, NULL);
	free(i);
	printf("delete 1, ret=%d\n", ret);
	print_list(listp);
	printf("\n");

	mcl_list_destroy(listp, MCL_DEF_FREE_FN);
	return 1;
}

