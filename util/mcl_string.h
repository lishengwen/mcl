#ifndef __MCL_STRING_H__
#define __MCL_STRING_H__

#include <string.h>

typedef struct mcl_string_s {
	unsigned int *_len;
	char _ptr[0];
} mcl_string;

#define MCL_STR_PTR(__strp) (char *)(__strp->_ptr)
#define MCL_STR_LEN(__strp) (*__strp->_len)

mcl_string *mcl_string_alloc(const char *strp, int len);
void mcl_string_free(mcl_string *mcl_strp);
mcl_string *mcl_string_copy(mcl_string *src);
void mcl_trim(mcl_string *strp);

#endif
