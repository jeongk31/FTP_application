#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdbool.h>

#define PORT 21
#define BUFFER_SIZE 1024

// Credential structure
typedef struct {
    char username[256];
    char password[256];
} Credential;

// load credentials
bool load_credentials(const char* filename, Credential credentials[], int* count) {
    FILE* file = fopen(filename, "r");
    if (!file) return false;

    char line[512];
    while (fgets(line, sizeof(line), file)) {
        if (sscanf(line, "%[^:]:%s", credentials[*count].username, credentials[*count].password) == 2) {
            (*count)++;
        }
    }

    fclose(file);
    return true;
}

// Handle client
void handleClient(int clientSocket, Credential credentials[], int credential_count) {
    char buffer[BUFFER_SIZE];
    strcpy(buffer, "220 FTP Server Ready\r\n");
    send(clientSocket, buffer, strlen(buffer), 0);

    char currentUsername[256] = "";
    bool authenticated = false;

    // Command handling loop
    while(1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        if (bytesReceived <= 0) break;

        // Check for USER command
        if (strncmp(buffer, "USER", 4) == 0) {
            char username[256];
            sscanf(buffer, "USER %s", username);
            strcpy(currentUsername, username);
            strcpy(buffer, "331 Username OK, need password\r\n");
            send(clientSocket, buffer, strlen(buffer), 0);
        }
        // Check for PASS command
        else if (strncmp(buffer, "PASS", 4) == 0) {
            char password[256];
            sscanf(buffer, "PASS %s", password);

            // Authenticate  user
            for (int i = 0; i < credential_count; i++) {
                if (strcmp(currentUsername, credentials[i].username) == 0 && strcmp(password, credentials[i].password) == 0) {
                    authenticated = true;
                    break;
                }
            }

            // Send response
            if (authenticated) {
                strcpy(buffer, "230 User logged in, proceed\r\n");
            } else {
                strcpy(buffer, "530 Not logged in\r\n");
            }
            send(clientSocket, buffer, strlen(buffer), 0);
        }
        // (for checking purposes. remove)
        else {
            send(clientSocket, buffer, bytesReceived, 0);
        }
    }

    close(clientSocket);
}


int main() {
    Credential credentials[100]; //n of users
    int credential_count = 0;
    load_credentials("details.txt", credentials, &credential_count);
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr;

    // Create server socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Bind server socket
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // Bind server address to socket
    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(serverSocket, 5) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("FTP Server started on port %d\n", PORT);

    // Accept incoming connections
    fd_set read_fds;
    int fd_max = serverSocket;

    FD_ZERO(&read_fds);
    FD_SET(serverSocket, &read_fds);

    // Accept incoming connections
    while(1) {
        clientSocket = accept(serverSocket, NULL, NULL);
        if (clientSocket < 0) {
            perror("Accept failed");
            continue;
        }

        // Fork new process to handle the client
        if (!fork()) {
            close(serverSocket);
            handleClient(clientSocket, credentials, credential_count);
            exit(0);
        }
        close(clientSocket);
    }

    return 0;
}
