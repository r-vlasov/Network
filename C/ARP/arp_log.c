#include "arp_log.h"

#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <unistd.h>


log_file arp_log_file;


int log_init(char *log_file)
{
    // logfile struct init 
    arp_log_file.log_stream = fopen(log_file, "wt");
    if ( arp_log_file.log_stream == NULL )
    {
        ABORT("Can't open log_file\n");
        return -1;
    }
    strcpy(arp_log_file.log_filename, log_file);
    arp_log_file.log_enabled = 1;
    INFO("Log file initiated successfully");
    return 0;
}   

static char* log_type_message(char type)
{
    switch(type)
    {
        case LOG_INFO:
            return LOG_INFO_STRING;
        case LOG_DEBUG:
            return LOG_DEBUG_STRING;
        case LOG_FATAL:
            return LOG_FATAL_STRING;
        default:
            return NULL;
    }
}

void arp_log(char type, const char *source, int line, char *message, ...)
{
    // log with timestamps
    time_t now;
    time(&now);
    struct tm *tm_now = gmtime(&now); 

	va_list arg_list;
	char buffer[LOG_MAX_MESSAGE_LENGTH];
	
	va_start(arg_list, message);
	vsnprintf(buffer, LOG_MAX_MESSAGE_LENGTH, message, arg_list);
	va_end(arg_list);
    
	fprintf(    arp_log_file.log_stream, "[%s]\t[%4d-%02d-%02d\t%02d:%02d:%02d]\t:\t%s[%d]\t%s\n",\
                log_type_message(type), tm_now->tm_year + 1900, tm_now->tm_mon + 1, tm_now->tm_mday, tm_now->tm_hour, \
                tm_now->tm_min, tm_now->tm_sec, source, line, buffer );
	
}