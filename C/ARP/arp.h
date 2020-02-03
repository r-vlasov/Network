#ifndef _ARP_H
#define _ARP_H


#define PROTO_ARP       0x0806
#define MAC_ADDRESS_LENGTH  6
#define IP_ADDRESS_LENGTH   4


struct __attribute__((packed)) arp_header {
    unsigned short hardware_type; // Ethernet - 1
    unsigned short protocol_type; // IPv4 - 0800
    unsigned char hardware_size;
    unsigned char protocol_size;
    unsigned short opcode;
    unsigned char sender_mac[MAC_ADDRESS_LENGTH];
    unsigned char sender_ip [IP_ADDRESS_LENGTH];
    unsigned char target_mac[MAC_ADDRESS_LENGTH];
    unsigned char target_ip [IP_ADDRESS_LENGTH];
};

// Ethernet
#define ETH2_HEADER_LEN 14
#define HW_TYPE         1
#define PROTOCOL_TYPE   0x800
#define ARP_REQUEST     0x01
#define ARP_REPLY       0x02
#define ETHERNET_FRAME  42



#endif