#include <util/mcl_array.h>
#include <stdio.h>


void print_array(mcl_array *arr)
{
	mcl_iter it = MCL_ITER_INITIALIZER;
	int *info = NULL;
	MCL_FOREACH_REVERSE(it, arr) {
		info = (int *)arr->iter_info(&it, arr);
		printf("%d ", *info);
	}
	printf("\n");
}

void print_size(mcl_array *arr)
{
	printf("arr cur_size: %d\n", mcl_array_size(arr));
}

void print_capacity(mcl_array *arr)
{
	printf("arr total_size: %d\n", mcl_array_capacity(arr));
}

int main(int argc, char *argv[])
{
	int i = 0;
	int *intp = NULL;
	int ret = 0;
	mcl_array *arr = mcl_array_new(100);
	print_array(arr);
	print_size(arr);
	print_capacity(arr);

	for (i = 0; i < 20; ++ i) {
		intp = (int *)malloc(sizeof(int));
		*intp = i;
		if (i % 2) {
			ret = mcl_array_insert(intp, 0, arr);
		}
		else {
			ret = mcl_array_preinsert(intp, 0, arr);
		}

		if (ret < 0) {
			printf("insert %d error, ret = %d\n", *intp, ret);
		}
		else {
			printf("insert %d ok, ret = %d\n", *intp, ret);
		}
	}

	print_size(arr);
	print_capacity(arr);

	intp = (int *)mcl_array_at(17, arr);
	printf("pos 17: %d\n", *intp);
	intp = (int *)mcl_array_at(0, arr);
	printf("pos 0: %d\n", *intp);
	print_array(arr);

	mcl_array_delete_indx(17, arr, MCL_DEF_FREE_FN);
	mcl_array_delete_obj(intp, arr, MCL_DEF_FREE_FN);
	print_array(arr);

	mcl_array_delete_range(2, 6, arr, MCL_DEF_FREE_FN);
	print_array(arr);

	intp = (int *)malloc(sizeof(int));
	*intp = 100;
	ret = mcl_array_append(intp, arr);
	print_array(arr);

	for (i = 1; i <= 10; ++ i) {
		intp = (int *)malloc(sizeof(int));
		*intp = i * 100;
		mcl_array_preappend(intp, arr);
	}
	print_array(arr);

	mcl_array_clean(arr, MCL_DEF_FREE_FN);
	print_array(arr);

	for (i = 1; i <= 10; ++ i) {
		intp = (int *)malloc(sizeof(int));
		*intp = i * 13;
		mcl_array_preinsert(intp, 0, arr);
	}
	print_size(arr);
	print_array(arr);

	mcl_array_delete_range(1, 4, arr, MCL_DEF_FREE_FN);
	print_array(arr);

	print_capacity(arr);
	mcl_array_tighten(arr);
	print_capacity(arr);

	mcl_array_destroy(arr, MCL_DEF_FREE_FN);

	return 1;
}

