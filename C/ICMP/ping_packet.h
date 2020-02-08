/* ICMP packet format */

#ifndef _PING_PACKET_H
#define _PING_PACKET_H


#define ICMP_ECHO           8
#define ICMP_NET_UNREACH	0	
#define RECV_TIMEOUT        1
struct icmp_packet {
    unsigned char type;
    unsigned char code;
    unsigned short chksum;
    union
    {
        
        struct
        {
            unsigned short id;
            unsigned short sequence;
        } echo;			/* echo datagram */
        unsigned int gateway;	/* gateway address */
        struct
        {
            unsigned short unused;
            unsigned short mtu;
        } frag;			/* path mtu discovery */
    } un;
    unsigned char data[64];
};

#endif 