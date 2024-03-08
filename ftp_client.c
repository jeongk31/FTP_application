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
    memset(buffer, 0, BUFFER_SIZE);
    read(*client_socket, buffer, BUFFER_SIZE);
    printf("%s", buffer);
}

// Send command to server
void send_command(int client_socket, const char *command) {
    char buffer[BUFFER_SIZE];
    sprintf(buffer, "%s\r\n", command);
    send(client_socket, buffer, strlen(buffer), 0);

    memset(buffer, 0, BUFFER_SIZE);
    read(client_socket, buffer, BUFFER_SIZE);
    printf("%s", buffer);
}

void send_port_command(int controlSocket) {
    int dataSocket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in dataAddr;
    memset(&dataAddr, 0, sizeof(dataAddr));
    dataAddr.sin_family = AF_INET;
    dataAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    dataAddr.sin_port = 0;

    bind(dataSocket, (struct sockaddr *)&dataAddr, sizeof(dataAddr));

    // Get port number
    socklen_t len = sizeof(dataAddr);
    getsockname(dataSocket, (struct sockaddr *)&dataAddr, &len);

    int port = ntohs(dataAddr.sin_port);
    unsigned char p1 = port / 256;
    unsigned char p2 = port % 256;

    unsigned char *ipParts = (unsigned char *)&dataAddr.sin_addr.s_addr;
    
    char command[255];
    sprintf(command, "PORT %d,%d,%d,%d,%d,%d", ipParts[0], ipParts[1], ipParts[2], ipParts[3], p1, p2);

    // PORT command to server
    send(controlSocket, command, strlen(command), 0);

    listen(dataSocket, 1); // Listen for connection
}


// Function to receive a file
void receive_file(int dataSocket, const char *filename) {
    FILE *file = fopen(filename, "wb");
    char buffer[BUFFER_SIZE];
    int bytesReceived;

    while ((bytesReceived = recv(dataSocket, buffer, BUFFER_SIZE, 0)) > 0) {
        fwrite(buffer, 1, bytesReceived, file);
    }

    fclose(file);
    close(dataSocket);
}

// Function to send a file
void send_file(int dataSocket, const char *filename) {
    FILE *file = fopen(filename, "rb");
    char buffer[BUFFER_SIZE];
    int bytesRead;

    while ((bytesRead = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        send(dataSocket, buffer, bytesRead, 0);
    }

    fclose(file);
    close(dataSocket);
}



int main() {
    const char *server_address = "127.0.0.1";
    int client_socket;
    connect_to_server(server_address, &client_socket);
    char buffer[BUFFER_SIZE];

    char command[256];
    int dataSocket = -1;

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
        }
        else if (strcmp(command, "LIST") == 0 || strncmp(command, "RETR", 4) == 0 || strncmp(command, "STOR", 4) == 0) {
            // LIST command is handled here
            send_port_command(client_socket); // Prepare data connection
            send_command(client_socket, command);

            if (strncmp(command, "RETR", 4) == 0) {
                char filename[256];
                sscanf(command + 5, "%s", filename);
                                
                memset(buffer, 0, BUFFER_SIZE);
                read(client_socket, buffer, BUFFER_SIZE); // check 200
                printf("%s", buffer);
                
                receive_file(dataSocket, filename); // handle receive_file
                
                do {
                    memset(buffer, 0, BUFFER_SIZE);
                    read(client_socket, buffer, BUFFER_SIZE);
                    printf("%s", buffer);
                } while (!strstr(buffer, "226 Transfer completed."));
            }

            else if (strncmp(command, "STOR", 4) == 0) {
                char filename[256];
                sscanf(command + 5, "%s", filename);
                send_file(dataSocket, filename); // handle send_file
            }
        }
        else if (strncmp(command, "!LIST", 5) == 0) {
            system("ls");
        }
        else {
            send_command(client_socket, command);
        }
    }

    close(client_socket);
    return 0;
}
