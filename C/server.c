#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <netinet/in.h>
#include <string.h>

#define PORT 8081
#define BACKLOG_MAX 5

int create_socket()
{
    int server_sockfd;
    if ((server_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Cannot create socket");
        exit(EXIT_FAILURE);
    }
    return server_sockfd;
}

struct sockaddr_in bind_socket(int sock)
{
    struct sockaddr_in address;
    
    /* convert integers from machine perfomance to network perfomance */
    memset((char*)&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(PORT);

    if (bind(sock,(struct sockaddr *)&address,sizeof(address)) < 0) 
    { 
        perror("Bind failed"); 
        exit(EXIT_FAILURE);
    }
    return address;
}

void listen_socket(int sock)
{
    if (listen(sock, BACKLOG_MAX) < 0)
    {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
}

int socket_accept(int server_sock, struct sockaddr_in addr)
{
    int new_socket;
    int addrlen = sizeof(addr);
    if ((new_socket = accept(server_sock, (struct sockaddr*) &addr, &addrlen)) < 0)
    {
        perror("In accept");
        exit(EXIT_FAILURE);
    }
    return new_socket;
}

int main()
{
    int server_socketfd = create_socket();
    struct sockaddr_in address_structure = bind_socket(server_socketfd);
    listen_socket(server_socketfd);

    char sended_data[] = "HTTP/1.1 200 OK\nContent-Type: \
                          text/plain\nContent-Length: 12\n\nHello world!";
    int new_sock;
    while (1)
    {
        printf("===Waiting for new connections===\n");
        new_sock = socket_accept(server_socketfd, address_structure);
        printf("===New socket was accepted===\n");
        char buffer[1000] = {0};
        read(new_sock, buffer, 1000);
        printf("Sended from client: %s\n", buffer);
        write(new_sock, sended_data, sizeof(sended_data));
        printf("===Hello was sent===\n");
        close(new_sock); 
    }
}
