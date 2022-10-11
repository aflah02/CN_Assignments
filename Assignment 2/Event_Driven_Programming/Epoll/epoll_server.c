#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>  
#include <sys/socket.h>  
#include <netinet/in.h>
#include <arpa/inet.h>
#include <inttypes.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <sys/types.h>
#include <strings.h>

long long int computeFactorial(int n)
{
    long long int fact = 1;
    for (int i = 1; i <= n; i++)
        fact *= i;
    return fact;
}

int main(){

    int numberOfConnections = 0;

    int mainSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (mainSocket < 0){
        perror("Main Socket Could Not be Created");
        exit(1);
    }

    struct epoll_event epollEvents[10];
    struct epoll_event event;

    int epollFD = epoll_create(10);



    if (epollFD < 0){
        perror("Epoll Could Not be Created");
        exit(1);
    }

    FILE *fp = fopen("EPollResults.txt", "w+");

    if (fp == NULL){
        perror("File Could Not be Opened");
        exit(1);
    }

    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    int bindResponse = bind(mainSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

    if (bindResponse < 0){
        perror("Bind Failed");
        exit(1);
    }

    int listenResponse = listen(mainSocket, 20);

    if (listenResponse < 0){
        perror("Listen Failed");
        exit(1);
    }

    printf("Server is Listening...\n");

    int maxFDs = 10;

    event.events = EPOLLIN;
    event.data.fd = mainSocket;

    if (epoll_ctl(epollFD, EPOLL_CTL_ADD, mainSocket, &event) < 0){
        perror("Epoll Control Failed");
        exit(1);
    }

    struct sockaddr_in clientData;
    socklen_t clientDataLength = sizeof(clientData);
    
    while(1){

        // if (numberOfConnections == 10){
        //     printf("10 Connections Reached. Server is Closing...\n");
        //     break;
        // }

        int epollResponse = epoll_wait(epollFD, epollEvents, maxFDs, -1);

        if (epollResponse < 0){
            perror("Epoll Wait Failed");
            exit(1);
        }

        for (int i = 0; i < epollResponse; i++){
            if (epollEvents[i].events & EPOLLIN){
                if (epollEvents[i].data.fd == mainSocket){
                    int newConnection = accept(mainSocket, (struct sockaddr*)&clientData, &clientDataLength);

                    if (newConnection < 0){
                        perror("Accept Failed");
                        exit(1);
                    }
                    printf("Accepted\n");
                    event.events = EPOLLIN;
                    event.data.fd = newConnection;

                    if (epoll_ctl(epollFD, EPOLL_CTL_ADD, newConnection, &event) < 0){
                        perror("Epoll Control Failed");
                        exit(1);
                    }
                }
                else{
                    char recieving_buffer[1000];
                    bzero(recieving_buffer, 1000);
                    int readResponse = read(epollEvents[i].data.fd, recieving_buffer, 1000);

                    if (readResponse < 0){
                        perror("Read Failed");
                        exit(1);
                    }
                    if(readResponse==0){
                        // FD_CLR(i, &allFDs);
                        if (epoll_ctl(epollFD, EPOLL_CTL_DEL, epollEvents[i].data.fd, &event) < 0){
                            perror("Epoll Control Failed");
                            exit(1);
                        }
                        close(epollEvents[i].data.fd);
                    }
                    else{
                        int value_recieved = atoi(recieving_buffer);
                        // printf("Value Recieved: %d \n", value_recieved);
                        long long int factorial = computeFactorial(value_recieved);
                        char sending_buffer[1000];
                        // sprintf(sending_buffer, "%lld", factorial);
                        // int writeResponse = write(i, sending_buffer, 1000);

                        // if (writeResponse < 0){
                        //     perror("Write Failed");
                        //     exit(1);
                        // }

                        // send(i, sending_buffer, 1000, 0);
                        
                        getpeername(epollEvents[i].data.fd, (struct sockaddr*)&clientData, &clientDataLength);
                        
                        printf("Client IP Address: %s, Port: %d, Value: %d, Factorial: %lld\n", inet_ntoa(clientData.sin_addr), ntohs(clientData.sin_port), value_recieved, factorial);

                        // Write to File

                        fprintf(fp, "Client IP Address: %s, Port: %d, Value: %d, Factorial: %lld\n", inet_ntoa(clientData.sin_addr), ntohs(clientData.sin_port), value_recieved, factorial);

                        fflush(fp);

                        snprintf(sending_buffer, 1000, "%lld", factorial);
                        send(epollEvents[i].data.fd, sending_buffer, 1000, 0);
    
                    }
                }
            }
        }
    }

    fclose(fp);
    close(mainSocket);
    return 0;
}