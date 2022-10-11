#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <inttypes.h>
#include <fcntl.h> 
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

sem_t semaphore;

FILE *fD;

struct threadData{
    int client_socket_fd;
    struct sockaddr_in client_sockaddr_in;
    socklen_t client_sockaddr_in_length;
};

long long int computeFactorial(int n)
{
    long long int fact = 1;
    for (int i = 1; i <= n; i++)
        fact *= i;
    return fact;
}

void * tFUN(void *args){
    struct threadData *data = (struct threadData *)args;
    int client_socket_fd = data->client_socket_fd;
    struct sockaddr_in client_sockaddr_in = data->client_sockaddr_in;
    socklen_t client_sockaddr_in_length = data->client_sockaddr_in_length;
    char buffer[1000];
    int i = 1;
    while (i <= 20){
        // Read
        read(client_socket_fd, buffer, 1000);
        printf("Received from Client: %s \n", buffer);
        printf("\n");

        // Compute Factorial
        int number = atoi(buffer);
        long long int result = computeFactorial(number);
        // printf("result: %lld \n", result);
        // Write
        char message[1000];
        snprintf(message, 1000, "%lld", result);
        printf("Sending Message: %s \n", message);
        write(client_socket_fd, message, sizeof(message));

        i++;
        
        // Client IP
        char client_ip[1000];
        inet_ntop(AF_INET, &(client_sockaddr_in.sin_addr), client_ip, 1000);

        // Client Port
        int client_port = ntohs(client_sockaddr_in.sin_port);

        // Avoid Race Condition
        sem_wait(&semaphore);

        // Write to File
        char write_buffer_for_file[1000];
        // sprintf(write_buffer_for_file, "Client IP is - %s, Client Port is - %d, Client Sent Number - %d, Factorial of Number is - %lld \n", client_ip, client_port, number, result);  
        snprintf(write_buffer_for_file, 1000, "Client IP is - %s, Client Port is - %d, Client Sent Number - %d, Factorial of Number is - %lld \n", client_ip, client_port, number, result);     
        printf("Writing to File: %s \n", write_buffer_for_file);
        fprintf(fD,"%s",write_buffer_for_file);

        
        sem_post(&semaphore);

        // // Write to Client
        // char write_buffer_for_client[1000];
        // // sprintf(write_buffer_for_client, "Factorial is %lld \n", result);
        // snprintf(write_buffer_for_client, 1000, "Factorial is %lld \n", result);
        // printf("Writing to Client: %s \n", write_buffer_for_client);
        // write(client_socket_fd, write_buffer_for_client, sizeof(write_buffer_for_client));
    }

    // Close Socket
    int close_res = close(client_socket_fd);

    if (close_res == -1){
        printf("Failed to Close Client Socket");
        exit(0);
    }

    pthread_exit(NULL);
}

int main(){
    pthread_t threads[10];
    // Open File
    fD = fopen("threadedResults.txt", "w+");
    sem_init(&semaphore, 0, 1);
    // Server Socket
    struct sockaddr_in server_sockaddr_in;
    server_sockaddr_in.sin_family = AF_INET;
    server_sockaddr_in.sin_port = htons(8080);
    server_sockaddr_in.sin_addr.s_addr = htonl(INADDR_ANY);

    // Server Socket Creation
    int server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_socket_fd == -1){
        perror("Failed to Create Server Socket");
        exit(1);
    }

    socklen_t server_sockaddr_in_length = sizeof(server_sockaddr_in);

    // Bind Server Socket
    int bind_response = bind(server_socket_fd, (struct sockaddr*)&server_sockaddr_in, server_sockaddr_in_length);

    if (bind_response < 0){
        perror("Failed to Bind Server Socket");
        exit(1);
    }

    // Listen to Server Socket
    int listen_response = listen(server_socket_fd, 10);

    if (listen_response < 0){
        perror("Failed to Listen to Server Socket");
        exit(1);
    }

    printf("Server is Listening...\n");

    int i = 1;
    while (i <= 10){
        struct sockaddr_in client_sockaddr_in;
        socklen_t client_sockaddr_in_length = sizeof(client_sockaddr_in);

        // Accept Client Socket
        int client_socket_fd = accept(server_socket_fd, (struct sockaddr*)&client_sockaddr_in, &client_sockaddr_in_length);
        struct threadData *data = (struct threadData*)malloc(sizeof(struct threadData));
        data->client_socket_fd = client_socket_fd;
        data->client_sockaddr_in = client_sockaddr_in;
        data->client_sockaddr_in_length = client_sockaddr_in_length;

        if (client_socket_fd < 0){
            perror("Failed to Accept Client Socket");
            exit(1);
        }

        // Create Thread and run on it

        pthread_create(&threads[i],NULL,tFUN,(void*)data);

        i++;
    }

    // Join Threads
    for (int i = 1; i <= 10; i++){
        pthread_join(threads[i], NULL);
    }

    // // Close Socket
    // close(server_socket_fd);

    // Close File
    fclose(fD);

    // Delete Semaphore
    sem_destroy(&semaphore);

    return 0;
}