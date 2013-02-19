#ifndef __MCL_COMMON_H__
#define __MCL_COMMON_H__

#include <stdlib.h>

#if defined(__cplusplus)
#define MCL_HEADER_BEGIN	extern "C" {
#define MCL_HEADER_END		}
#else
#define MCL_HEADER_BEGIN
#define MCL_HEADER_END
#endif 

typedef enum mcl_data_type_e {
	MCL_T_INT = 1,
	MCL_T_STR = 2,
	MCL_T_ARR = 3,
	MCL_T_MAP = 4,
} mcl_data_type;

#define MCL_IF_NOT_RET(expr, ret) \
	do { \
		if (!(expr)) { \
			return ret; \
		} \
	} while(0) 

#define MCL_DEF_FREE_FN	free

typedef void (*mcl_free_fn_t)(void *);

#endif
