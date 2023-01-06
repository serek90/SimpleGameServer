/*
*       client TCP connections using IPv4 and IPv6
*
*
*       author: jseroczy
*/


#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/types.h>
#include <string.h>


int main()
{
        printf("Hello from game client\n");

        const char *host = "::1";
        const char *port = "2222";

        // 1. getddrinfo()
        struct addrinfo *addr_init = calloc(1, sizeof(struct addrinfo));
        addr_init->ai_family = AF_INET6; // IPv4
        addr_init->ai_socktype = SOCK_STREAM; //TCP

        struct addrinfo *client_addr;
        if(getaddrinfo(host, port, addr_init, &client_addr))
        {
                perror("Failed\n");
                exit(1);
        }
        printf("Get addrinfo succes!\n");

        //2.socket()
        printf("Assinging socket\n");
        int sockfd = socket(client_addr->ai_family, client_addr->ai_socktype, client_addr->ai_protocol);
        if(sockfd == -1)
        {
                perror("Failed to assign the socket\n");
                exit(1);
        }
        printf("Socket assigned!\n");

        //3.connect()
        printf("Connecting socket ...\n");
        if(connect(sockfd, client_addr->ai_addr, client_addr->ai_addrlen) == -1)
        {
                perror("Failed to connect to server\n");
                exit(1);
        }
        printf("Succesfully connected to the server\n");

        //4.send() recv() in while loop
        //const int buffer_size = 1024;
        while(1)
        {
                char buffer[] = "Hello server\n";
                int ret = send(sockfd, buffer, strlen(buffer), 0);
                if(ret < 1)
                {
                        perror("Send failed\n");
                        exit(1);
                }
                sleep(20);
        }
        //5.close()

        return 0;
}
