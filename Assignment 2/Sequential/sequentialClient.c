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

int main(){
    // Client Socket
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
    fflush(stdout);
    int i = 1;
    while (i<=20){
        printf("Waiting for Server to Send Data \n");
        
        // Send Message
        char message[100];
        snprintf(message, 100, "%d", i);
        printf("Sending Message: %s \n", message);
        write(client_socket_fd, message, sizeof(message));

        // Receive Message
        char buffer[100];
        read(client_socket_fd, buffer, 100);
        printf("Received from Server: %s \n", buffer);
        printf("\n");

        i++;
    }

    // Close Socket
    close(client_socket_fd);
}