#include "parseargs.h"
#include "logstream.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

config_struct configure_ping;

static int pop_type_arg(char** args) {
    if (args[0] != NULL && args[0][0] == '-') {
        switch (args[0][1]) {
            case HOST_LITERAL:
                DEBUG("Parsing -h flag");
                strcpy(configure_ping.host, args[1]);
                DEBUG("Host: %s", configure_ping.host);
                return HOST;
            case ATTEMPTS_LITARAL:
                DEBUG("Parsing -a flag");
                configure_ping.attempts = atoi(args[1]);
                DEBUG("Attempts: %d", configure_ping.attempts);
                return ATTEMPTS;
            default:
                DEBUG("Underfined flag");
        }
    }
    return NOFLAGS;
}


config_struct parse_args(char** args) {
    args += 1;
    while (pop_type_arg(args) != NOFLAGS) {
        args += 2;
    };
    return configure_ping;
}