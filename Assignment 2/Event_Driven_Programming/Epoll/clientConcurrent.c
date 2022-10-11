#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>  
#include <sys/socket.h>  
#include <netinet/in.h>
#include <arpa/inet.h>
#include <inttypes.h>
#include <pthread.h>
#include <semaphore.h>

void *tFun(void *arg){

    int client_socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (client_socket_fd == -1){
        printf("Failed to Create Client Socket");
        exit(0);
    }

    // Server Socket
    
    struct sockaddr_in server_sockaddr_in;
    
    server_sockaddr_in.sin_family = AF_INET;
    server_sockaddr_in.sin_port = htons(8080);
    server_sockaddr_in.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Connect to Server
    int connect_response = connect(client_socket_fd, (struct sockaddr*)&server_sockaddr_in, sizeof(server_sockaddr_in));

    if (connect_response == -1){
        printf("Failed to Connect to Server");
        exit(0);
    }

    int i = 1;

    while (i<=20){
        printf("Waiting for Server to Send Data \n");
        // printf("%d \n", i);
        // Send Message
        char message[1000];
        snprintf(message, 1000, "%d", i);
        printf("Sending Message: %s \n", message);
        write(client_socket_fd, message, sizeof(message));

        // Receive Message
        char buffer[1000];
        read(client_socket_fd, buffer, 1000);
        printf("Received from Server: %s \n", buffer);
        printf("\n");

        i++;
    }

    // Close Socket
    close(client_socket_fd);

    return NULL;    

}

int main(){

    pthread_t clients[10];
    for(int i=0;i<10;i++){
        pthread_create(&clients[i],NULL,tFun,&i);
    }
    for(int i=0;i<10;i++){
        pthread_join(clients[i],NULL);
    }
    
    return 0;
}