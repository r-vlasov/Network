// thank you Mikko Korkallo from stackoverflow:) // 
#include "arp_log.h"
#include "arp.h"


#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/if_arp.h>
#include <arpa/inet.h> 
#include <unistd.h>


 
static int get_if_info(const char *ifname, uint32_t *ip, char *mac, int *ifindex)
{
    DEBUG("get_if_info for %s", ifname);
    struct ifreq ifr;
    int sd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
    if (sd == -1) {
        FATAL("socket()");
        return -1;
    }
    if (strlen(ifname) > (IFNAMSIZ - 1)) {
        FATAL("Too long interface name, MAX=%i\n", IFNAMSIZ - 1);
        return -1;
    }

    strcpy(ifr.ifr_name, ifname);

    DEBUG("Get interface index using name");
    if (ioctl(sd, SIOCGIFINDEX, &ifr) == -1) {
        FATAL("SIOCGIFINDEX");
        return -1;
    }
    *ifindex = ifr.ifr_ifindex;
    DEBUG("interface index is %d\n", *ifindex);

    DEBUG("Get MAC address of the interface");
    if (ioctl(sd, SIOCGIFHWADDR, &ifr) == -1) {
        FATAL("SIOCGIFINDEX");
        return -1;
    }

    DEBUG("Copy mac address to output");
    memcpy(mac, ifr.ifr_hwaddr.sa_data, MAC_ADDRESS_LENGTH);

    DEBUG("get_if_info OK");
    DEBUG("Clean up temporary socket");
    close(sd);
    return 0;
}

int bind_arp(int ifindex, int *fd)
{
    DEBUG("bind_arp: ifindex=%i", ifindex);
    DEBUG("Submit request for a raw socket descriptor");
    *fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
    if (*fd < 1) 
    {
        FATAL("Failed socket()");
        return -1;
    }

    DEBUG("Binding to ifindex %i", ifindex);
    struct sockaddr_ll sll;
    memset(&sll, 0, sizeof(struct sockaddr_ll));
    sll.sll_family = AF_PACKET;
    sll.sll_ifindex = ifindex;
    if (bind(*fd, (struct sockaddr*) &sll, sizeof(struct sockaddr_ll)) < 0) 
    {
        FATAL("Failed bind()");
        return -1;
    }
    return 0;
}



int send_arp(int fd, int ifindex, const unsigned char *src_mac, uint32_t src_ip, uint32_t dst_ip)
{
    unsigned char buffer[ETHERNET_FRAME];
    memset(buffer, 0, sizeof(buffer));

    // special for packet sockets
    struct sockaddr_ll socket_address;
    socket_address.sll_family = AF_PACKET;
    socket_address.sll_protocol = htons(ETH_P_ARP);
    socket_address.sll_ifindex = ifindex;
    socket_address.sll_hatype = htons(ARPHRD_ETHER);
    socket_address.sll_pkttype = PACKET_BROADCAST;
    socket_address.sll_halen = MAC_ADDRESS_LENGTH;
    socket_address.sll_addr[6] = 0x00;
    socket_address.sll_addr[7] = 0x00;

    struct ethhdr *eth_request = (struct ethhdr *) buffer;  // the header of eth frame
    struct arp_header *arp_request = (struct arp_header *) (buffer + ETH2_HEADER_LEN);

    //Broadcast
    memset(eth_request->h_dest, 0xff, MAC_ADDRESS_LENGTH);

    //Target MAC zero
    memset(arp_request->target_mac, 0x00, MAC_ADDRESS_LENGTH);

    DEBUG("Set source mac to our MAC address");
    memcpy(eth_request->h_source, src_mac, MAC_ADDRESS_LENGTH);
    memcpy(arp_request->sender_mac, src_mac, MAC_ADDRESS_LENGTH);
    memcpy(socket_address.sll_addr, src_mac, MAC_ADDRESS_LENGTH);

    DEBUG("Setting protocol of the packet");
    eth_request->h_proto = htons(ETH_P_ARP);

    DEBUG("Creating ARP request");
    arp_request->hardware_type = htons(HW_TYPE);
    arp_request->protocol_type = htons(ETH_P_IP);
    arp_request->hardware_size = MAC_ADDRESS_LENGTH;
    arp_request->protocol_size = IP_ADDRESS_LENGTH;
    arp_request->opcode = htons(ARP_REQUEST);

    DEBUG("Copy IP address to arp_request");
    memcpy(arp_request->sender_ip, &src_ip, sizeof(uint32_t));
    memcpy(arp_request->target_ip, &dst_ip, sizeof(uint32_t));

    DEBUG("Sending arp packet");
    ssize_t ret = sendto(fd, buffer, ETHERNET_FRAME, 0, (struct sockaddr *) &socket_address, sizeof(socket_address));
    if (ret == -1) 
    {
        FATAL("Failed to send arp-packet");
        return -1;
    }
    else 
        DEBUG("Success sending arp-packet");
    return 0;
}

int read_arp(int fd)
{
    // There should be the cycle with N tries
    DEBUG("read_arp");
    unsigned char buffer[ETHERNET_FRAME];
    ssize_t length = recvfrom(fd, buffer, ETHERNET_FRAME, 0, NULL, NULL);
    int index;
    if (length == -1) 
    {
        FATAL("recvfrom()");
        return -1;
    }
    struct ethhdr *rcv_response = (struct ethhdr *) buffer;
    struct arp_header *arp_response = (struct arp_header *) (buffer + ETH2_HEADER_LEN);
    if (ntohs(rcv_response->h_proto) != PROTO_ARP)
    {
        FATAL("Not an ARP packet");
        return -1;
    }
    if (ntohs(arp_response->opcode) != ARP_REPLY) 
    {
        FATAL("Not an ARP reply");
        return -1;
    }
    DEBUG("received ARP len=%ld", length);
    struct in_addr sender;
    memset(&sender, 0, sizeof(struct in_addr));
    memcpy(&sender.s_addr, arp_response->sender_ip, sizeof(uint32_t));
    DEBUG("Sender IP: %s", inet_ntoa(sender));

    fprintf(stdout, "Sender MAC: %02X:%02X:%02X:%02X:%02X:%02X",
          arp_response->sender_mac[0],
          arp_response->sender_mac[1],
          arp_response->sender_mac[2],
          arp_response->sender_mac[3],
          arp_response->sender_mac[4],
          arp_response->sender_mac[5]);
    
    return 0;

}

/*
 *
 * Sample code that sends an ARP who-has request on
 * interface <ifname> to IPv4 address <ip>.
 * Returns 0 on success.
 */
int test_arping(const char *ifname, const char *ip) 
{
    int ret = -1;
    uint32_t dst = inet_addr(ip);
    if (dst == 0 || dst == 0xffffffff) 
    {
        FATAL("Invalid source IP\n");
        return 1;
    }

    int src;
    int ifindex;
    char mac[MAC_ADDRESS_LENGTH];
    if (get_if_info(ifname, &src, mac, &ifindex)) 
    {
        FATAL("get_if_info failed, interface %s not found or no IP set?", ifname);
        return -1;
    }
    int arp_fd;
    if (bind_arp(ifindex, &arp_fd)) {
        FATAL("Failed to bind_arp()");
        return -1;
    }

    if (send_arp(arp_fd, ifindex, mac, src, dst)) {
        FATAL("Failed to send_arp");
        return -1;
    }
    
    int r = read_arp(arp_fd);
    if (r == 0) 
    {
        DEBUG("Got reply, break out");
    }
    else
    {
        FATAL("Сould not get an answer, break out");
    }
    if (arp_fd) 
    {
        close(arp_fd);
        arp_fd = 0;
    }
    return 0;
}

int main(int argc, const char **argv) {
    int ret = -1;
    if (argc != 3) {
        printf("Usage: %s <INTERFACE> <DEST_IP>\n", argv[0]);
        return 1;
    }
    const char *ifname = argv[1];
    const char *ip = argv[2];
    log_init("mylogfile.txt");
    return test_arping(ifname, ip);
}
