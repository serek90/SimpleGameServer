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
#include "betGameProtocol.h"

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

        //4.send() recv()
        struct BEGASEP_header *mess_header = calloc(1, sizeof(struct BEGASEP_header));
        mess_header->Ver = 1;
        mess_header->Len = sizeof( struct BEGASEP_header);
        mess_header->Type = BEGASEP_OPEN;
        mess_header->Client_id = 0;
        int ret = send(sockfd, mess_header, sizeof(struct BEGASEP_header), 0);
        if(ret < 1)
        {
                perror("Send failed\n");
                exit(1);
        }

        struct BEGASEP_accept *mess_accept = calloc(1, sizeof(struct BEGASEP_accept));
        ret = recv(sockfd, mess_accept, sizeof(struct BEGASEP_accept), 0);
        if(ret > 0)
        {
                printf("Received %d bythes \n", ret);
                printf("ver: %d, len: %d, type: %d, client_id: %d\n", mess_accept->header.Ver,
                                                                  mess_accept->header.Len,
                                                                  mess_accept->header.Type,
                                                                  mess_accept->header.Client_id);
                printf("Max num: %d, min num: %d\n", mess_accept->lowNumRange, mess_accept->upNumRange);

        }

        /* (3) Receive message BEGASEP_BET */
        struct BEGASEP_betting *mess_bet = calloc(1, sizeof(struct BEGASEP_betting));
        mess_bet->header.Ver = 1;
        mess_bet->header.Len = sizeof( struct BEGASEP_betting);
        mess_bet->header.Type = BEGASEP_BET;
        mess_bet->header.Client_id = mess_accept->header.Client_id;
        mess_bet->bettingNum = 20; //JSJS hardcode

        ret = send(sockfd, mess_bet, sizeof(struct BEGASEP_betting), 0);
        if(ret < 1)
        {
                perror("Send failed\n");
                exit(1);
        }

        /* (4) Send message BEGASEP_RESULT */
        struct BEGASEP_result *mess_result = calloc(1, sizeof(struct BEGASEP_result));
        ret = recv(sockfd, mess_result, sizeof(struct BEGASEP_result), 0);
        if(ret > 0)
        {
                printf("Received %d bythes\n", ret);
                printf("ver: %d, len: %d, type: %d, client_id: %d\n", mess_result->header.Ver,
                                                                  mess_result->header.Len,
                                                                  mess_result->header.Type,
                                                                  mess_result->header.Client_id);
                printf("The great number is: %d and you: %s!!!", mess_result->winNum, mess_result->status ? "won" : "lose");

        }

        /* 5. close socket */
        close(sockfd);

        return 0;
}
