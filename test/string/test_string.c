#include <util/mcl_string.h>
#include <stdio.h>

int main()
{
	const char * strp = "  hello world  ";

	mcl_string * mstrp = mcl_string_alloc(strp, strlen(strp));
	printf("%s\n", MCL_STR_PTR(mstrp));

	mcl_trim(mstrp);
	printf("%s\n", MCL_STR_PTR(mstrp));

	mcl_string * cpstrp = mcl_string_copy(mstrp);
	printf("%s\n", MCL_STR_PTR(cpstrp));

	mcl_string_free(mstrp);
	mcl_string_free(cpstrp);

	return 1;
}


