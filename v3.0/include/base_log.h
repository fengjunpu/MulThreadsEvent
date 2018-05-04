#ifndef _BASE_LOG_H_
#define _BASE_LOG_H_
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

enum {
	DEBUG_LEVEL_LOG = 0,
	INFO_LEVEL_LOG,	
	WARN_LEVEL_LOG,
	ERROR_LEVEL_LOG,
};

static int current_log_level;

#define SET_LOG_LEVEL(x) do{if( ((x) < DEBUG_LEVEL_LOG) || ((x) > ERROR_LEVEL_LOG) ){ break;} current_log_level = (x);}while(0);

static void _log_print(int level, char *buf, size_t len);

static void _log_helper(int level, const char *fromat, va_list ap);

void log_debugx(const char * format, ...);

void log_infox(const char * format, ...);

void log_warnx(const char * format, ...);

void log_errx(const char * format, ...);


#endif
