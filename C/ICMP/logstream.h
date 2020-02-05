#ifndef __LOGSTREAM_H
#define __LOGSTREAM_H

#include  <stdarg.h>

#define DEBUG_MODE

#define LOGFILENAME     "ping.log"


#define DEBUG_TYPE   0
#define DEBUG_STRING    "[DEBUG]"

#define INFO_TYPE    1
#define INFO_STRING    "[INFO]"

#define FATAL_TYPE   2
#define FATAL_STRING    "[FATAL]"

#ifdef DEBUG_MODE
#define DEBUG(message, ...)     logfile_write(DEBUG_STRING, message, ##__VA_ARGS__)
#else
#define DEBUG(message, ...)
#endif

#define INFO(message, ...)      logfile_write(INFO_STRING, message, ##__VA_ARGS__)
#define FATAL(message, ...)     logfile_write(FATAL_STRING, message, ##__VA_ARGS__)

#define LOGOUT_BUFFER_LENGTH    100

int logfile_init();
int logfile_write(char* prefix, char* message, ...);  // outstream in logfile
int logfile_close();
#endif