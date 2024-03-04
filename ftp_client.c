#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define FTP_PORT 21
#define BUFFER_SIZE 1024

// Connect to server
void connect_to_server(const char *server_address, int *client_socket) {
    struct sockaddr_in serverAddr;

    // Create socket
    *client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (*client_socket < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Connect to server
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(FTP_PORT);

    if (inet_pton(AF_INET, server_address, &serverAddr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    if (connect(*client_socket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Connection Failed");
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];
    read(*client_socket, buffer, BUFFER_SIZE);
    printf("Server response: %s", buffer);
}

// Send command to server
void send_command(int client_socket, const char *command) {
    char buffer[BUFFER_SIZE];
    sprintf(buffer, "%s\r\n", command);
    send(client_socket, buffer, strlen(buffer), 0);

    memset(buffer, 0, BUFFER_SIZE);
    read(client_socket, buffer, BUFFER_SIZE);
    printf("Server response: %s", buffer);
}


int main() {
    const char *server_address = "127.0.0.1";
    int client_socket;
    connect_to_server(server_address, &client_socket);

    char command[256];

    // Send commands to server
    while (1) {
        printf("ftp> ");
        fgets(command, sizeof(command), stdin);

        size_t len = strlen(command);
        if (len > 0 && command[len - 1] == '\n') {
            command[len - 1] = '\0';
        }
        
        if (strcmp(command, "QUIT") == 0) {
            send_command(client_socket, "QUIT");
            break;
        } else {
            send_command(client_socket, command);
        }
    }

    close(client_socket);
    return 0;
}
