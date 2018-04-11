#ifndef _BASE_LOG_H_
#define _BASE_LOG_H_
#include <stdio.h>
#include <stdarg.h>

#define _DEBUG_LEVEL_LOG 0
#define _INFO_LEVEL_LOG 1
#define _WARN_LEVEL_LOG 2
#define _ERROR_LEVEL_LOG 3

static void _log_print(int level, char *buf, size_t len);

static void _log_helper(int level, const char *fromat, va_list ap);
	
void log_debugx(const char * format, ...);

void log_infox(const char * format, ...);

void log_warnx(const char * format, ...);

void log_errx(int errno, const char * format, ...);


#endif
