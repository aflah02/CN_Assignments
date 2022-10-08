#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <inttypes.h>
#include <fcntl.h> // for open
#include <unistd.h> // for close

long long int computeFactorial(int n)
{
    long long int fact = 1;
    for (int i = 1; i <= n; i++)
        fact *= i;
    return fact;
}

// Main Function
int main(){
    struct sockaddr_in server_sockaddr_in;
    // Make File
    FILE *fp;
    // Open File
    fp = fopen("forkedResults.txt", "w+");
    
    // Socket Creation
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1){
        perror("Failed to Create Server Socket");
        exit(1);
    }

    server_sockaddr_in.sin_family = AF_INET;
    server_sockaddr_in.sin_addr.s_addr = htonl(INADDR_ANY);
    server_sockaddr_in.sin_port = htons(8080);

    int bind_response = bind(socket_fd, (struct sockaddr*)&server_sockaddr_in, sizeof(server_sockaddr_in));

    if (bind_response < 0){
        perror("Failed to Bind Server Socket");
        exit(1);
    }

    int listen_response = listen(socket_fd, 0);

    if (listen_response < 0){
        perror("Failed to Listen to Server Socket");
        exit(1);
    }

    printf("Server is Listening...\n");
    int i = 0;
    printf("Waiting for Client to Connect \n");
    
    int client_socket_fd;
    for (int x = 0; x < 10; x++){
        struct sockaddr_in client_sockaddr_in;
        socklen_t client_sockaddr_in_length = sizeof(client_sockaddr_in);
    
        client_socket_fd = accept(socket_fd, (struct sockaddr*)&client_sockaddr_in, &client_sockaddr_in_length);
        if (client_socket_fd < 0){
            printf("Failed to Accept Client Socket");
            exit(0);
        }

        int client_port = ntohs(client_sockaddr_in.sin_port);
        printf("Client Port: %d \n", client_port);

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(client_sockaddr_in.sin_addr), client_ip, INET_ADDRSTRLEN);
        printf("Client IP: %s \n", client_ip);

        //fork
        if(fork() == 0){
            
            close(socket_fd);
            char read_buffer[1000];
            for(int i = 0; i < 20; i++){
                //child
                bzero(read_buffer,1000);
                read(client_socket_fd, read_buffer, 1000);
                printf("Client Sent: %s \n", read_buffer);

                char write_buffer_for_file[1000];
                char write_buffer_for_client[1000];

                int number = atoi(read_buffer);

                long long int result = computeFactorial(number);

                printf("Result: %lld \n", result);


                sprintf(write_buffer_for_file, "Client IP is - %s, Client Port is - %d, Client Sent Number - %d, Factorial of Number is - %lld \n", client_ip, client_port, number, result);       
                printf("Writing to File: %s \n", write_buffer_for_file);
                fprintf(fp,"%s",write_buffer_for_file);


                sprintf(write_buffer_for_client, "Factorial is %lld \n", result);
                printf("Writing to Client: %s \n", write_buffer_for_client);
                write(client_socket_fd, write_buffer_for_client, sizeof(write_buffer_for_client));
            }
            exit(0);
        }
    }
    // while (i < 20){

    //     char read_buffer[1000];
    //     read(client_socket_fd, read_buffer, 1000);
    //     printf("Client Sent: %s \n", read_buffer);

    //     char write_buffer_for_file[1000];
    //     char write_buffer_for_client[1000];

    //     int number = atoi(read_buffer);

    //     long long int result = computeFactorial(number);

    //     printf("Result: %lld \n", result);


    //     sprintf(write_buffer_for_file, "Client IP is - %s, Client Port is - %d, Client Sent Number - %d, Factorial of Number is - %lld \n", client_ip, client_port, number, result);       
    //     printf("Writing to File: %s \n", write_buffer_for_file);
    //     fprintf(fp,"%s",write_buffer_for_file);


    //     sprintf(write_buffer_for_client, "Factorial is %lld \n", result);
    //     printf("Writing to Client: %s \n", write_buffer_for_client);
    //     write(client_socket_fd, write_buffer_for_client, sizeof(write_buffer_for_client));

    //     i++;

    // }

    fclose(fp);
    close(client_socket_fd);

    return 0;
}