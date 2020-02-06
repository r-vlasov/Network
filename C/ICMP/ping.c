#include "parseargs.h"
#include "logstream.h"
#include "ping_packet.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

struct in_addr* dns_host_to_ip(char* hostname) {
    INFO("Trying to convert hostname to IP-address");
    struct hostent *hp;
    struct in_addr **ip;

    if (!(hp = gethostbyname(hostname))) {
	    FATAL("Failed to lookup dns");
        return NULL;
    }

    int i = 0;
	while ( !(*hp->h_addr_list) + i ) {
        i++;
    }
	INFO("received IP: %s ", inet_ntoa(*(struct in_addr *) (*hp->h_addr_list + i)));
    return *hp->h_addr_list + i;
}

int set_raw_socket(char* hostname, struct sockaddr_in* sockaddr) {
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP); 
    if(sockfd < 0)
    {
        FATAL("Failed to socket()");
        return -1;
    }

    // describe socket that works with ip's protocols
    sockaddr->sin_family = AF_INET;
    sockaddr->sin_port = 53;
    sockaddr->sin_addr = *(dns_host_to_ip(hostname));  
    return sockfd;
}


/* Fill the ICMP packet */
unsigned short checksum(void *b, int len) {    
    unsigned short *buf = b; 
    unsigned int sum=0; 
    unsigned short result; 
  
    for ( sum = 0; len > 1; len -= 2 ) 
        sum += *buf++; 
    if ( len == 1 ) 
        sum += *(unsigned char*)buf; 
    sum = (sum >> 16) + (sum & 0xFFFF); 
    sum += (sum >> 16); 
    result = ~sum; 
    return result; 
}



int send_packet(int ping_sockfd, struct sockaddr* ping_addr, int seqnumber) {
    struct icmp_packet ping_pckt;
    bzero(&ping_pckt, sizeof(ping_pckt)); 
    ping_pckt.type = ICMP_ECHO;	
    ping_pckt.code = ICMP_NET_UNREACH;
    ping_pckt.un.echo.id = getpid();
    ping_pckt.un.echo.sequence = seqnumber;
    ping_pckt.chksum = checksum(&ping_pckt, sizeof(ping_pckt));
    if ( sendto(ping_sockfd, &ping_pckt, sizeof(ping_pckt), 0,  (struct sockaddr*) ping_addr,  sizeof(*ping_addr)) <= 0 ) 
    { 
        FATAL("Packet Sending Failed!");
        return -1;  
    }
    return seqnumber++;
}

int main(int argc, char** argv) {
    logfile_init();
    config_struct query = parse_args(argv);
    fprintf(stderr, "%d", sizeof(int));

    struct sockaddr_in sockaddr;
    memset(&sockaddr, 0, sizeof(struct sockaddr_in));
    int sock = set_raw_socket(query.host, &sockaddr);
    
    // seqnumber init
    srand(time(NULL));   // Initialization, should only be called once.
    int r = rand(); 
    send_packet(sock, &sockaddr, r);  
    logfile_close();
}