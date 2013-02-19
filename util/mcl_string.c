#include <mcl_string.h>
#include <stdlib.h>

mcl_string *mcl_string_alloc(const char *strp, int len)
{
	mcl_string *retp = (mcl_string *)malloc(sizeof(*retp) + len + 1);
	retp->_len = (unsigned int *)malloc(sizeof(unsigned int));
	*retp->_len = len;
	memcpy(retp->_ptr, strp, len);
	retp->_ptr[len] = 0;

	retp->_ref = 1;

	return retp;
}

static inline void do_free(mcl_string *strp)
{
	if (strp) {
		free(strp->_len);
		free(strp);
	}
}

void mcl_string_free(mcl_string *strp)
{
	if (!strp) return;

	-- strp->_ref;

	if (!strp->_ref) { 
		do_free(strp);
	}
}

mcl_string *mcl_string_copy(mcl_string *src)
{
	if (!src) {
		return NULL;
	}

	mcl_string *dest = mcl_string_alloc(MCL_STR_PTR(src), MCL_STR_LEN(src));

	return dest;
}

static inline is_space(char c)
{
	return c == ' ' || c == '\t';
}

void mcl_trim(mcl_string *strp)
{
	if (!strp) {
		return;
	}

	char * src = MCL_STR_PTR(strp);
	char *head, *tail;

	for (head = src; is_space(*head); ++ head);

	for (tail = src + MCL_STR_LEN(strp) - 1; is_space(*tail); -- tail);

	while (head <= tail) {
		*src ++ = *head ++;
	}

	*src = '\0';

	MCL_STR_LEN(strp) = strlen(MCL_STR_PTR(strp));
}

void mcl_string_ref(mcl_string *strp)
{
	if (!strp) {
		return;
	}

	++ strp->_ref;
}

void mcl_string_deref(mcl_string *strp)
{
	mcl_string_free(strp);
}

int mcl_string_cmp(mcl_string *one, mcl_string *another)
{
	const char *one_str = (const char *)MCL_STR_PTR(one);
	const char *another_str = (const char *)MCL_STR_PTR(another);

	return strcmp(one_str, another_str);
}

