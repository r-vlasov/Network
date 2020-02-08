#include "parseargs.h"
#include "logstream.h"
#include "ping_packet.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <time.h>


// timestamps
struct timeval stop, start;
struct timeval begin, end;

int success_response = 0;
struct in_addr* dns_host_to_ip(char* hostname) {
    INFO("Trying to convert hostname to IP-address");
    struct hostent *hp;

    if (!(hp = gethostbyname(hostname))) {
	    FATAL("Failed to lookup dns");
        return NULL;
    }

    int i = 0;
	while ( !(*hp->h_addr_list) + i ) {
        i++;
    }
	INFO("received IP: %s ", inet_ntoa(*(struct in_addr *) (*hp->h_addr_list + i)));
    return (struct in_addr *) *hp->h_addr_list + i;
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
    struct in_addr* addr = dns_host_to_ip(hostname);
    if (addr == NULL) {
        return -1;
    }
    sockaddr->sin_addr = *addr;
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

int send_packet(int ping_sockfd, struct sockaddr_in* ping_addr, int seqnumber) {
    struct icmp_packet ping_pckt;
    bzero(&ping_pckt, sizeof(ping_pckt)); 
    ping_pckt.type = ICMP_ECHO;	
    ping_pckt.code = ICMP_NET_UNREACH;
    ping_pckt.un.echo.id = getpid();
    ping_pckt.un.echo.sequence = seqnumber;
    ping_pckt.chksum = checksum(&ping_pckt, sizeof(ping_pckt));
    // fix send time
    gettimeofday(&start, NULL);
    if ( sendto(ping_sockfd, &ping_pckt, sizeof(ping_pckt), 0,  (struct sockaddr*) ping_addr,  sizeof(*ping_addr)) <= 0 ) { 
        FATAL("Packet Sending Failed!");
        return -1;  
    }
    return ++seqnumber;
}

int receive_packet(int ping_sockfd, struct sockaddr_in* ping_addr, int icmp_seq) {
    struct icmp_packet ping_pckt;
    int addrlen = sizeof(struct sockaddr); 
    if ( recvfrom(ping_sockfd, &ping_pckt, sizeof(ping_pckt), 0,  (struct sockaddr*) ping_addr, &addrlen) <= 0 )  { 
        FATAL("Packet receive failed!\n");
        gettimeofday(&stop, NULL);
        return -1; 
    }
    else {
        // fix rev time
        gettimeofday(&stop, NULL);
        // rtt - round-trip time
        fprintf(stderr, "%ld bytes from %s, icmp_sec = %d, rtt = %lu us\n", sizeof(ping_pckt), inet_ntoa(((struct sockaddr_in *)ping_addr)->sin_addr), \
                        icmp_seq, ((stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec)/1000);
        success_response++;
        return 0;
    }
}

int ping_loop(int attempts, int seqnumber, int sock, struct sockaddr_in* sockaddr) {
    int icmp_seq = 1;
    int pnumb = attempts;

    //sock timeout create
    struct timeval tv_out;
    tv_out.tv_sec = RECV_TIMEOUT; 
    tv_out.tv_usec = 0; 
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv_out, sizeof tv_out); 

    gettimeofday(&begin, NULL);
    while (attempts--) {
        seqnumber = send_packet(sock, sockaddr, seqnumber);
        if (seqnumber > 0) {
            if (!receive_packet(sock, sockaddr, icmp_seq)) {
                INFO("Success ping request-response number #%d", icmp_seq);
            }
            else {
                fprintf(stdout, "Failed, icmp_sec = %d\n", icmp_seq);
            }
        }
        else {
            return -1;
        }
        icmp_seq++;
    }
    gettimeofday(&end, NULL);
    printf("\n===ping statistics===\n"); 
    printf("\n%d packets sent, %d packets received, %f percent packet loss. Total time: %ld ms.\n\n",\
    pnumb, success_response, ((pnumb - success_response)/pnumb) * 100.0,\
    ((end.tv_sec - begin.tv_sec) * 1000000 + end.tv_usec - begin.tv_usec)/1000);  
    return 0;
}


int main(int argc, char** argv) {
    logfile_init();
    config_struct query = parse_args(argv);

    struct sockaddr_in sockaddr;
    memset(&sockaddr, 0, sizeof(struct sockaddr_in));
    int sock = set_raw_socket(query.host, &sockaddr);
    if (sock == -1) {
        fprintf(stdout, "ping: Name or service not known\n");
        logfile_close();
        return -1;
    }

    // seqnumber init
    srand(time(NULL));   // Initialization, should only be called once.
    int r = rand(); 
    ping_loop(query.attempts, r, sock, &sockaddr);
    logfile_close();
    return 0;
}