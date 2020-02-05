#include "logstream.h"
#include "parseargs.h"

#include <stdio.h>
int main(int argc, char** argv) {
    logfile_init();
    parse_args(argv);    
}