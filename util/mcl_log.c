#include <mcl_log.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#define MAX_LOG_TIME_LEN	30
#define MAX_LOG_BUFF_LEN	256
#define MAX_LOG_MODE_LEN	8
static char *g_log_dirname = "";

static void make_filepath(char *output, int size, const char *filename)
{
	int len = -1;
	if (strlen(g_log_dirname)) {
		len = snprintf(output, size, "%s/%s.log", g_log_dirname, filename);	
	}
	else {
		len = snprintf(output, size, "%s.log", filename);	
	}
	output[len] = '\0';
}

mcl_log *mcl_log_new(const char *log_name)
{
	FILE *fp = NULL;
	mcl_log *logger = (mcl_log *)malloc(sizeof(*logger));
	char file_path[512];

	make_filepath(file_path, sizeof(file_path), log_name);
	if (!strlen(file_path)) {
		fprintf(stderr, "log path error for name %s\n", log_name);
		return NULL;
	}

	fp = fopen(file_path, "a");
	if (!fp) {
		perror("open");
		return NULL;
	}

	logger->_fp = fp;
	logger->_lv = LV_ALL;

	return logger;
}

static void make_time(char *timestr, int size)
{
	time_t t;
	(void *)time(&t);
	strftime(timestr, size, "%Y-%m-%d %H:%M:%S", localtime(&t));
}

static void make_mode(char *modestr, int size, int lv)
{
	int ret = 0;
	switch (lv) {
		case LV_DEBUG:
			ret = snprintf(modestr, size, "DEBUG");
			break;
		case LV_INFO:
			ret = snprintf(modestr, size, "INFO");
			break;
		case LV_WARN:
			ret = snprintf(modestr, size, "WARN");
		case LV_ERROR:
			ret = snprintf(modestr, size, "ERROR");
			break;
		case LV_FATAL:
			ret = snprintf(modestr, size, "FATAL");
			break;
		default:
			ret = snprintf(modestr, size, "UNKONWN");
			break;
	}

	modestr[ret] = '\0';
}

void mcl_log_info(mcl_log *logger, mcl_log_level level, const char *file, int line, const char *fmt, ...)
{
	va_list args;
	FILE *fp = NULL;
	char timestr[MAX_LOG_TIME_LEN];
	char logbuff[MAX_LOG_BUFF_LEN];
	char modestr[MAX_LOG_MODE_LEN];

	if (!logger) return;

	if (level < logger->_lv) {
		printf("level forbiden\n");	
		return;	
	}
	if (!fmt) {
		printf("null log content\n");
		return;
	}

	fp = logger->_fp;
	if (!fp) {
		return;
	}

	make_time(timestr, sizeof(timestr));

	make_mode(modestr, sizeof(modestr), level);

	va_start(args, fmt);
	(void *)vsnprintf(logbuff, sizeof(logbuff), fmt, args);
	va_end(args);

	if (file) {
		fprintf(fp, "[%s] [%s] [file: %s, line: %d] %s\n", 
				timestr, modestr, file, line, logbuff);
	}
	else {
		fprintf(fp, "[%s] [%s] %s\n", 
				timestr, modestr, logbuff);
	}

	fflush(fp);
}

void mcl_log_set_level(mcl_log *logger, mcl_log_level lv)
{
	if (!logger) return;

	logger->_lv = lv;
}

void mcl_log_destroy(mcl_log *logger)
{
	if (logger->_fp) {
		fclose(logger->_fp);
	}

	free(logger);
}

