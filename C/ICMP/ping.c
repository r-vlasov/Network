#include "parseargs.h"
#include "logstream.h"

#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <netdb.h>

#define h_addr h_addr_list[0]

int dns_host_to_ip(char* hostname , char* ip) {
    INFO("Trying to convert hostname to IP-address");
}


int main(int argc, char** argv) {
    logfile_init();
    config_struct query = parse_args(argv);
    char ip[200];
    dns_host_to_ip(query.host, ip);
    logfile_close();
}