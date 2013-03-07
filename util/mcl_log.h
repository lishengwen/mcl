#ifndef __MCL_LOG_H__
#define __MCL_LOG_H__

#include <mcl_common.h>
#include <stdio.h>
#include <stdlib.h>

MCL_HEADER_BEGIN

typedef enum mcl_log_level_s {
	LV_ALL   = 1,
	LV_DEBUG = 2,
	LV_INFO  = 3,
	LV_WARN  = 4,
	LV_ERROR = 5,
	LV_FATAL = 6,
	LV_NONE  = 100,
} mcl_log_level;

typedef struct mcl_log_s {
	FILE *_fp;
	int _mode;
	mcl_log_level _lv;
} mcl_log;

mcl_log *mcl_log_new(const char *log_name);

void mcl_log_destroy(mcl_log *logger);

void mcl_log_info(mcl_log *logger, mcl_log_level level, const char *file, int line, const char *fmt, ...);

void mcl_log_set_level(mcl_log *logger, mcl_log_level lv);

void mcl_log_set_dir(const char *dirname);

#define MCLOG_DEBUG(__logger, __fmt...) \
	mcl_log_info(__logger, LV_DEBUG, __FILE__, __LINE__, __fmt)

#define MCLOG_INFO(__logger, __fmt...) \
	mcl_log_info(__logger, LV_INFO, NULL, -1, __fmt)

#define MCLOG_WARN(__logger, __fmt...) \
	mcl_log_info(__logger, LV_WARN, NULL, -1, __fmt)

#define MCLOG_ERROR(__logger, __fmt, __arg) \
	mcl_log_info(__logger, LV_ERROR, __FILE__, __LINE__, __fmt, __arg)

#define MCLOG_FATAL(__logger, __fmt, __arg) do {\
	mcl_log_info(__logger, LV_FATAL, __FILE__, __LINE__, __fmt, __arg); \
	abort(); \
} while(0)


MCL_HEADER_END

#endif
