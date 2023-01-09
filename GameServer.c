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
#include <pthread.h>
#include <stdatomic.h>
#include "betGameProtocol.h"

#define MAX_NUM 25
#define MIN_NUM 15
#define NTHREADS 40
#define PROGRAM_VER 1

struct client
{
        int fd; //file descriptor
        int id;
};

atomic_int endLottery;
uint32_t winning_num;

/*******************************
*net thread function
*******************************/
pthread_t netThread[NTHREADS];
atomic_int netThreadCtr = 0;
struct client client[NTHREADS];

void *netThreadFunc(void *arg)
{
        struct client *server = (struct client*)arg;
        struct sockaddr client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd;
        struct BEGASEP_header *mess_header = calloc(1, sizeof(struct BEGASEP_header));
        struct BEGASEP_accept *mess_accept = calloc(1, sizeof(struct BEGASEP_accept));
        struct BEGASEP_betting *mess_bet = calloc(1, sizeof(struct BEGASEP_betting));
        struct BEGASEP_result *mess_result = calloc(1, sizeof(struct BEGASEP_result));
        
        while(1)
        {

                /* Accept incoming connection */
                printf("Accepting a new connection\n");

                client_fd = accept(server->fd, &client_addr, &client_len);
                if(client_fd == -1)
                {
                        perror("Failed with accepted\n");
                        exit(1);
                }
                printf("Connection accepted\n");
                netThreadCtr++;

                /* (1) Receive message BEGASEP_OPEN from client */
                int ret = recv(client_fd, mess_header, sizeof(struct BEGASEP_header), 0);
                if(ret > 0)
                {
                        printf("Received %d bythes from client % d\n", ret, server->id);
                        printf("ver: %d, len: %d, type: %d, client_id: %d\n", mess_header->Ver,
                                                                          mess_header->Len,
                                                                          mess_header->Type,
                                                                          mess_header->Client_id);
                }

                /* (2) Send message BEGASEP_ACCEPT */
                mess_accept->header.Ver = PROGRAM_VER;
                mess_accept->header.Len = sizeof(struct BEGASEP_accept);
                mess_accept->header.Type = BEGASEP_ACCEPT;
                mess_accept->header.Client_id = server->id;
                mess_accept->lowNumRange = MIN_NUM;
                mess_accept->upNumRange = MAX_NUM;
                ret = send(client_fd, mess_accept, sizeof(struct BEGASEP_accept), 0);
                if(ret < 1)
                {
                        perror("Send failed\n");
                        exit(1);
                }

                /* (3) Receive message BEGASEP_BET */
                ret = recv(client_fd, mess_bet, sizeof(struct BEGASEP_betting), 0);
                if(ret > 0)
                {
                        printf("Received %d bythes from client % d\n", ret, server->id);
                        printf("ver: %d, len: %d, type: %d, client_id: %d\n", mess_bet->header.Ver,
                                                                          mess_bet->header.Len,
                                                                          mess_bet->header.Type,
                                                                          mess_bet->header.Client_id);
                        printf("Betting: %d\n", mess_bet->bettingNum);
                }

                /* wait for lottery to finish */
                while(endLottery){}

                /* (4) Send message BEGASEP_RESULT */
                mess_result->header.Ver = PROGRAM_VER;
                mess_result->header.Len = sizeof(struct BEGASEP_result);
                mess_result->header.Type = BEGASEP_RESULT;
                mess_result->header.Client_id = server->id;
                mess_result->winNum = winning_num;
                mess_result->status = (winning_num == mess_bet->bettingNum);
                ret = send(client_fd, mess_result, sizeof(struct BEGASEP_result), 0);
                if(ret < 1)
                {
                        perror("Send failed\n");
                        exit(1);
                }

                printf("DEBUG: Thread finish connection\n");
                /* Close client socket */
                close(client_fd);
        }
        
        free(mess_header);
        free(mess_accept);
        free(mess_bet);
        free(mess_result);
}

/******************************
*Lottery thread
******************************/
pthread_t lotteryThread;

void *lotteryThreadFunc(void *arg)
{
        while(1)
        {
                /* if there is a one connected netThear start lottery */
                if(netThreadCtr)
                {
                        printf("DEBUG: Start lottery\n");
                        endLottery = 1;
                        sleep(15);
                        winning_num  = rand()%MAX_NUM + MIN_NUM;
                        printf("%d\n", winning_num);
                        endLottery = 0;
                        netThreadCtr = 0;
                }
        }
}

/*****************************
* main thread
*****************************/
int main()
{
        srand(time(NULL));

        const char *host = "127.0.0.1";
        const char *port = "2222";

        /* 1. getddrinfo() */
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


        /* 2. Open socket */
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
        /* 3. Bind socket */
        printf("Binding socket ...\n");
        if(bind(sockfd, server_addr->ai_addr, server_addr->ai_addrlen) == -1)
        {
                perror("Failed to bind addres\n");
                exit(1);
        }
        printf("Binding succes\n");

        /* Start lottery thread */
        pthread_create(&lotteryThread, NULL, lotteryThreadFunc, NULL);

        /* 4. Listen for incoming connections */
        printf("Listening...\n");
        if(listen(sockfd,1) == -1)
        {
                perror("Failed\n");
                exit(1);
        }


        /* Create network threads */
        for(int i = 0; i < NTHREADS; i++)
        {
                client[i].id = i;
                client[i].fd = sockfd;
                int result = pthread_create(&netThread[i], NULL, netThreadFunc, (void *) &client[i]);
                if (result != 0) {
                        perror("Could not create thread!!!\n");
                        exit(1);
                }

        }

        /* main loop */
        while(1){}

        /* join network threads */
        for(int i = 0; i < NTHREADS; i++)
                pthread_join(netThread[i], NULL);

        /*7. close and free allocated memory */
        freeaddrinfo(server_addr);
        close(sockfd);

        return 0;
}
