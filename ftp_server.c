#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdbool.h>
#include <arpa/inet.h>

#define PORT 21
#define BUFFER_SIZE 1024

// credential structure
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

int setup_data_connection(char *clientIp, int dataPort) {
    int dataSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (dataSocket < 0) {
        perror("Data socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in dataAddr;
    memset(&dataAddr, 0, sizeof(dataAddr));
    dataAddr.sin_family = AF_INET;
    dataAddr.sin_port = htons(dataPort);

    if (inet_pton(AF_INET, clientIp, &dataAddr.sin_addr) <= 0) {
        perror("Invalid address");
        exit(EXIT_FAILURE);
    }

    if (connect(dataSocket, (struct sockaddr *)&dataAddr, sizeof(dataAddr)) < 0) {
        perror("Data port");
        exit(EXIT_FAILURE);
    }

    return dataSocket;
}


// Handle client
void handleClient(int clientSocket, Credential credentials[], int credential_count) {
    char buffer[BUFFER_SIZE];
    char msg[256];
    strcpy(msg, "220 Service ready for new user.\r\n");
    send(clientSocket, msg, strlen(msg), 0);

    char currentUsername[256] = "";
    bool authenticated = false;

    char clientIp[256];
    int dataPort;

    // Command handling loop
    while(1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        if (bytesReceived <= 0) break;

        // Check for PORT command
        if (strncmp(buffer, "PORT", 4) == 0) {
            int h1, h2, h3, h4, p1, p2;
            sscanf(buffer, "PORT %d,%d,%d,%d,%d,%d", &h1, &h2, &h3, &h4, &p1, &p2);
            dataPort = (p1 * 256) + p2;
            sprintf(clientIp, "%d.%d.%d.%d", h1, h2, h3, h4);
            strcpy(msg, "200 PORT command successful.\r\n");
            send(clientSocket, msg, strlen(msg), 0);
        }
        // Check for STOR command
        else if (strncmp(buffer, "STOR", 4) == 0) {
            char filename[256];
            int dataSocket = setup_data_connection(clientIp, dataPort);
            sscanf(buffer + 5, "%s", filename);

            FILE *file = fopen(filename, "wb");
            if (file == NULL) {
                strcpy(msg, "Could not create file.\r\n");
                send(clientSocket, msg, strlen(msg), 0);
            } else {
                strcpy(msg, "150 File status okay; about to open data connection.\r\n");
                send(clientSocket, msg, strlen(msg), 0);
                
                int bytes;
                char fileBuffer[BUFFER_SIZE];
                while ((bytes = recv(dataSocket, fileBuffer, BUFFER_SIZE, 0)) > 0) {
                    fwrite(fileBuffer, 1, bytes, file);
                }
                fclose(file);
                close(dataSocket);

                strcpy(msg, "226 Transfer complete.\r\n");
                send(clientSocket, msg, strlen(msg), 0);
            }
        }
        // Check for LIST command
        else if (strncmp(buffer, "LIST", 4) == 0) {
            int dataSocket = setup_data_connection(clientIp, dataPort);
            
            strcpy(msg, "150 File status okay; about to open data connection.\r\n");
            send(clientSocket, msg, strlen(msg), 0);

            FILE *fp = popen("ls", "r");
            char line[256];
            while (fgets(line, sizeof(line), fp) != NULL) {
                send(dataSocket, line, strlen(line), 0);
            }
            pclose(fp);

            close(dataSocket);

            strcpy(msg, "226 Transfer complete.\r\n");
            send(clientSocket, msg, strlen(msg), 0);
        }
        // Check for USER command
        else if (strncmp(buffer, "USER", 4) == 0) {
            char username[256];
            sscanf(buffer, "USER %s", username);
            strcpy(currentUsername, username);

            strcpy(msg, "331 Username OK, need password\r\n");
            send(clientSocket, msg, strlen(msg), 0);
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
                strcpy(msg, "230 User logged in, proceed\r\n");
            } else {
                strcpy(msg, "530 Not logged in.\r\n");
            }
            send(clientSocket, msg, strlen(msg), 0);
        }
        // Check for RETR command
        else if (strncmp(buffer, "RETR", 4) == 0) {
            char filename[256];
            sscanf(buffer + 5, "%s", filename);

            // open the file
            FILE *file = fopen(filename, "rb");
            if (file == NULL) {
                strcpy(msg, "550 No such file or directory.\r\n");
                send(clientSocket, msg, strlen(msg), 0);
            } else {
                strcpy(msg, "150 File status okay; about to open data connection.\r\n");
                send(clientSocket, msg, strlen(msg), 0);

                int dataSocket = setup_data_connection(clientIp, dataPort);
                char fileBuffer[BUFFER_SIZE];
                int bytesRead;

                // read file and send it over data connection
                while ((bytesRead = fread(fileBuffer, 1, BUFFER_SIZE, file)) > 0) {
                    send(dataSocket, fileBuffer, bytesRead, 0);
                }
                fclose(file);
                close(dataSocket);

                strcpy(msg, "226 Transfer completed.\r\n");
                send(clientSocket, msg, strlen(msg), 0);
            }
        }
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

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // bind server address to socket
    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(serverSocket, 5) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("FTP Server started on port %d\n", PORT);
    
    fd_set read_fds;
    int fd_max = serverSocket;

    FD_ZERO(&read_fds);
    FD_SET(serverSocket, &read_fds);

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
