#include "base_log.h"

void _log_print(int level, char *msg)
{
	const char *loglevelstr;
	switch(level) {
		case _DEBUG_LEVEL_LOG:
			loglevelstr = "debug";
			break;
		case _INFO_LEVEL_LOG:
			loglevelstr = "info";
			break;
		case _WARN_LEVEL_LOG:
			loglevelstr = "warn";
			break;
		case _ERROR_LEVEL_LOG:
			loglevelstr = "err";
			break;
	}
	(void)fprintf(stderr,"[%s] %s\n",loglevelstr,msg);
}

void _log_helper(int level, const char *format, va_list ap)
{
	char buf[1024];
	size_t len = sizeof(buf);
	if(format != NULL) {
		vsnprintf(buf,len,format,ap);
	} else {
		buf[0] = '\0';
	}
	_log_print(level, buf);
}
	
void log_debugx(const char * format, ...)
{
	va_list ap;
	va_start(ap,format);
	_log_helper(_DEBUG_LEVEL_LOG,format,ap);
	va_end(ap);
}

void log_infox(const char * format, ...)
{
	va_list ap;
	va_start(ap,format);
	_log_helper(_INFO_LEVEL_LOG,format,ap);
	va_end(ap);
}

void log_warnx(const char * format, ...)
{
	va_list ap;
	va_start(ap,format);
	_log_helper(_WARN_LEVEL_LOG,format,ap);
	va_end(ap);
}

void log_errx(int errno, const char * format, ...)
{
	va_list ap;
	va_start(ap,format);
	_log_helper(_ERROR_LEVEL_LOG,format,ap);
	va_end(ap);
}
