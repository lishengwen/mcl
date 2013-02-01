#include <mcl_string.h>
#include <stdlib.h>

mcl_string *mcl_string_alloc(const char *strp, int len)
{
	mcl_string *retp = (mcl_string *)malloc(sizeof(*retp) + len + 1);
	retp->_len = (unsigned int *)malloc(sizeof(unsigned int));
	*retp->_len = len;
	memcpy(retp->_ptr, strp, len);
	retp->_ptr[len] = 0;

	return retp;
}

void mcl_string_free(mcl_string *strp)
{
	if (strp) {
		free(strp->_len);
		free(strp);
	}
}

mcl_string *mcl_string_copy(mcl_string *src)
{
	if (src == NULL) {
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
	if (strp == NULL) {
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

