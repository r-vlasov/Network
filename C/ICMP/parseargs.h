#ifndef __PARSEARGS_H
#define __PARSEARGS_H

// Type of args
#define HOST_LITERAL        'h'
#define ATTEMPTS_LITARAL    'a'

#define NOFLAGS     -1
#define HOST        0
#define ATTEMPTS    1

typedef struct {
    char host[20];
    int attempts;
} config_struct;

config_struct parse_args(char** args);

#endif