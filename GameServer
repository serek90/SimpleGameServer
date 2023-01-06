/*
*       Server accept TCP connections using IPv4 and IPv6
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

#define MAX_NUM 5
#define MIN_NUM 1

int main()
{
        srand(time(NULL));

        const char *host = "127.0.0.1";
        const char *port = "2222";

        // 1. getddrinfo()
        struct addrinfo *addr_init = calloc(1, sizeof(struct addrinfo));
        addr_init->ai_family = AF_INET6; // IPv6
        addr_init->ai_socktype = SOCK_STREAM; //TCP
        addr_init->ai_flags = AI_PASSIVE;

        struct addrinfo *server_addr;
        if(getaddrinfo(0, port, addr_init, &server_addr))
        {
                perror("Failed\n");
                free(addr_init);
                exit(1);
        }
        printf("Get addrinfo succes!\n");
        free(addr_init);
        addr_init = NULL;


        //2. socket()
        printf("Assinging socket\n");
        int sockfd = socket(server_addr->ai_family, server_addr->ai_socktype, server_addr->ai_protocol);
        if(sockfd == -1)
        {
                perror("Failed to assign the socket\n");
                exit(1);
        }
        printf("Socket assigned!\n");

        /* activate dual stack configuration */
        int option = 0;
        if(setsockopt(sockfd, IPPROTO_IPV6, IPV6_V6ONLY, (void *)&option, sizeof(option)) == -1)
        {
                perror("Failed to activate doual stack config\n");
                exit(1);
        }
        //3. bind()
        printf("Binding socket ...\n");
        if(bind(sockfd, server_addr->ai_addr, server_addr->ai_addrlen) == -1)
        {
                perror("Failed to bind addres\n");
                exit(1);
        }
        printf("Binding succes\n");

        //4. listen()
        /* Listen for incoming connections */
        printf("Listening...\n");
        if(listen(sockfd,1) == -1)
        {
                perror("Failed\n");
                exit(1);
        }

        //5. accept()
        printf("Accepting a new connection\n");
        struct sockaddr client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(sockfd, &client_addr, &client_len);
        if(client_fd == -1)
        {
                perror("Failed with accepted\n");
                exit(1);
        }
        printf("Connection accepted\n");

        //6. send() recv()
        const int buffer_size = 1024;
        char buffer[buffer_size];
        int ret = recv(client_fd, buffer, buffer_size, 0);
        if(ret > 0)
        {
                printf("Received %d bythes:\n", ret);
                for(int i = 0; i < ret; i++)
                        putchar(buffer[i]);
        }
        sleep(5);

        /* main loop */
        while(1)
        {
                uint16_t number = rand()%MAX_NUM + MIN_NUM;
                printf("%d\n", number);
                int ret = recv(client_fd, buffer, buffer_size, 0);
                if(ret > 0)
                {
                        printf("Received %d bythes:\n", ret);
                        for(int i = 0; i < ret; i++)
                                putchar(buffer[i]);
                }
                sleep(5);
        }

        // 7. close()
        /* Close client socket */
        close(client_fd);
        /*free allocated memory */
        freeaddrinfo(server_addr);
        /* Clear resources */
        close(sockfd);

        return 0;
}
