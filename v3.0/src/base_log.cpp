#include "../include/base_log.h"

void _log_print(int level, char *msg)
{
	const char *loglevelstr;
	switch(level) {
		case DEBUG_LEVEL_LOG:
			loglevelstr = "DEBUG";
			break;
		case INFO_LEVEL_LOG:
			loglevelstr = "INFO";
			break;
		case WARN_LEVEL_LOG:
			loglevelstr = "WARN";
			break;
		case ERROR_LEVEL_LOG:
			loglevelstr = "ERROR";
			break;
	}
	        
	struct tm *t = NULL; 
	time_t timer = time(NULL);
	t = localtime(&timer);  
	FILE *fstream = level > WARN_LEVEL_LOG ? stderr : stdout;
	(void)fprintf(fstream,"[%s | %04d-%02d-%02d %02d:%02d:%02d] | %s\n",loglevelstr,\
				   t->tm_year+1900,t->tm_mon+1,t->tm_mday,t->tm_hour,t->tm_min,t->tm_sec,msg);
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
	if(current_log_level > DEBUG_LEVEL_LOG)
		return ;
	va_list ap;
	va_start(ap,format);
	_log_helper(DEBUG_LEVEL_LOG,format,ap);
	va_end(ap);
}

void log_infox(const char * format, ...)
{
	if(current_log_level > INFO_LEVEL_LOG)
		return ;
	va_list ap;
	va_start(ap,format);
	_log_helper(INFO_LEVEL_LOG,format,ap);
	va_end(ap);
}

void log_warnx(const char * format, ...)
{
	if(current_log_level > WARN_LEVEL_LOG)
		return ;
	va_list ap;
	va_start(ap,format);
	_log_helper(WARN_LEVEL_LOG,format,ap);
	va_end(ap);
}

void log_errx(const char * format, ...)
{
	if(current_log_level > ERROR_LEVEL_LOG)
		return ;
	va_list ap;
	va_start(ap,format);
	_log_helper(ERROR_LEVEL_LOG,format,ap);
	va_end(ap);
}
