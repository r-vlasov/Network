#include "logstream.h"

#include <stdio.h>
#include <stdarg.h>
#include <fcntl.h>


static FILE* logout_fd = 0;


int logfile_init() {
    logout_fd = fopen(LOGFILENAME, "wt");
    if (logout_fd == NULL) {
        fprintf(stderr, "Failed to open logout file\n");
        return -1;
    }
    INFO("Logout file initialized");
    return 0;
}


int logfile_write(char* prefix, char* message, ...) {
    va_list arg_list;
	char buffer[LOGOUT_BUFFER_LENGTH];
	
	va_start(arg_list, message);
	vsnprintf(buffer, LOGOUT_BUFFER_LENGTH, message, arg_list);
	va_end(arg_list);
    
	fprintf(logout_fd, "%s:%s\n", prefix, buffer);
    return 0;
}

int logfile_close() {
    fclose(logout_fd);
    return 0;
}