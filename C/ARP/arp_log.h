#ifndef _ARP_LOG_H
#define _ARP_LOG_H

#include <stdio.h>

#define LOG_INFO    0
#define LOG_DEBUG   1
#define LOG_FATAL   2

#define LOG_INFO_STRING     "INFO"
#define LOG_DEBUG_STRING    "DEBUG"
#define LOG_FATAL_STRING    "FATAL"

int log_init(char *config_file);

void arp_log(char level, const char *source, int line, char *message, ...);

#define INFO(message, ...)      arp_log(LOG_INFO, __FILE__, __LINE__, message, ##__VA_ARGS__)
#define DEBUG(message, ...)     arp_log(LOG_DEBUG, __FILE__, __LINE__, message, ##__VA_ARGS__)
#define FATAL(message, ...)     arp_log(LOG_FATAL, __FILE__, __LINE__, message, ##__VA_ARGS__)
#define ABORT(message)          fprintf(stderr, message)

#define MAX_FILE_PATH           20
#define LOG_MAX_MESSAGE_LENGTH  100
typedef struct log_file
{
    int log_enabled;
    FILE * log_stream;
    char log_filename[MAX_FILE_PATH];
} log_file;

#endif