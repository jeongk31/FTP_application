#include "client.h"

// receive a file
void receive_file(int dataSocket, const char *filename) {
    printf("Receiving file: %s\n", filename);
    FILE *file = fopen(filename, "wb");
    if (!file) {
        perror("Failed to open file on client");
        close(dataSocket);
        return;
    }

    char buffer[BUFFER_SIZE];
    int bytesReceived;

    while ((bytesReceived = recv(dataSocket, buffer, BUFFER_SIZE, 0)) > 0) {
		//printf("receive file read buffer: %s\n", buffer);
        fwrite(buffer, 1, bytesReceived, file);
    }

    if (bytesReceived < 0) {
        perror("Receive failed");
    }

    fclose(file);
    close(dataSocket);
    printf("File transfer complete.\n");
}


// send a file
void send_file(int dataSocket, const char *filename) {
    FILE *file = fopen(filename, "rb");

	if (!file) {
        send(dataSocket, "550 File not found.\r\n", strlen("550 File not found.\r\n"), 0);
        printf("550 File not found.\n");
		close(dataSocket);
        return;
    }
	else
	{

    	char buffer[BUFFER_SIZE];
    	int bytesRead;

    	while ((bytesRead = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
    	    send(dataSocket, buffer, bytesRead, 0);
    	}

    	fclose(file);
    	close(dataSocket);
	}
}



int	main(void)
{
	struct sockaddr_in	server_address;
	int					client_socket;
	int					data_socket = -1;
	char				buffer[BUFFER_SIZE];

	if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}

	memset(&server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(PORT);
	server_address.sin_addr.s_addr = inet_addr("127.0.0.1");

	if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
	{
		perror("connect failed");
		exit(EXIT_FAILURE);
	}
	// initial connection message
	printf("Hello!! Please Authenticate to run server commands\n");
    printf("1. type \"USER\" followed by a space and your username\n");
    printf("2. type \"PASS\" followed by a space and your password\n\n");
    
    printf("\"QUIT\" to close connection at any moment\n");
    printf("Once Authenticated\n");
    printf("this is the list of commands :\n");
    printf("\"STOR\" + space + filename |to send a file to the server\n");
    printf("\"RETR\" + space + filename |to download a file from the server\n");
    printf("\"LIST\" |to list all the files under the current server directory\n");
    printf("\"CWD\" + space + directory |to change the current server directory\n");
    printf("\"PWD\" to display the current server directory\n");
    printf("Add \"!\" before the last three commands to apply them locally\n\n");

	//wait for server to send response (initial connection)
	receiveResponse(client_socket);

	while (1)
	{
		printf("ftp> ");
		memset(buffer, 0, BUFFER_SIZE);
		fgets(buffer, BUFFER_SIZE, stdin);

		//parse input: remove newline
		buffer[strcspn(buffer, "\n")] = '\0';

		//compare & handle commands
		if (strncmp(buffer, "USER", 4) == 0 || strncmp(buffer, "PASS", 4) == 0)
		{
			write(client_socket, buffer, strlen(buffer));
			receiveResponse(client_socket);
		}
		else if (strcmp(buffer, "QUIT") == 0)
		{
			write(client_socket, "QUIT", strlen("QUIT"));
			receiveResponse(client_socket);
			break ;
		}
		else if (strncmp(buffer, "!LIST", 5) == 0 || strncmp(buffer, "!PWD", 4) == 0)
		{
			write(client_socket, buffer, strlen(buffer));

			char response[BUFFER_SIZE];
			memset(response, 0, BUFFER_SIZE);
			read(client_socket, response, BUFFER_SIZE);

			if (strncmp(response, "100", 3) == 0)
			{
				if (strncmp(buffer, "!LIST", 5) == 0)
					system("ls");
				else if (strncmp(buffer, "!PWD", 4) == 0)
				{
					system("pwd");
					printf(".");
				}
			}
			else
				printf("%s", response);
		}
		else if (strncmp(buffer, "!CWD", 4) == 0)
		{
			write(client_socket, buffer, strlen(buffer));

			char response[BUFFER_SIZE];
			memset(response, 0, BUFFER_SIZE);
			read(client_socket, response, BUFFER_SIZE);

			if (strncmp(response, "100", 3) == 0)
			{
				char *directory = getName(buffer);

				if (chdir(directory) == 0)
				{
					//directory change success
					printf(" 200 directory changed to %s.\n", directory);
				}
				else
					printf("550 Failed to change directory.\n");
				free(directory);
			}
			else
				printf("%s", response);
		}
		else if (strncmp(buffer, "PWD", 3) == 0)
		{
			write(client_socket, buffer, strlen(buffer));
			receiveResponse(client_socket);
		}
		
		else if (strncmp(buffer, "CWD", 3) == 0)
		{
			write(client_socket, buffer, strlen(buffer));
			receiveResponse(client_socket);
		}
		else if (strncmp(buffer, "LIST", 4) == 0 || strncmp(buffer, "RETR", 4) == 0 || strncmp(buffer, "STOR", 4) == 0)
		{
			// LIST command is handled here
            data_socket = send_port_command(client_socket);
			char	command[256];
			strncpy(command, buffer, 4);
			command[4] = '\0';
			printf("command: %s\n", command);
			printf("data socket: %d\n", data_socket);
			
            int status = send_command(client_socket, buffer);

            if (status == 1 && strncmp(command, "RETR", 4) == 0) {
                char filename[256];
                sscanf(buffer + 5, "%s", filename);

				//send_command(client_socket, buffer);//++++
                                
                memset(buffer, 0, BUFFER_SIZE);//++++
                //read(client_socket, buffer, BUFFER_SIZE); // check 200
                printf("main: %s", buffer);//++++

				struct sockaddr_in serverAddr;
				socklen_t addrSize = sizeof(serverAddr);
				int dataTransferSocket = accept(data_socket, (struct sockaddr *)&serverAddr, &addrSize);
				if (dataTransferSocket < 0) {
					perror("Failed to accept data connection");
					exit(EXIT_FAILURE);
				}
                printf("before receive file: %s\n", buffer);
                receive_file(dataTransferSocket, filename); // handle receive_file

				char serverResponse[BUFFER_SIZE];
                
                do {
					memset(serverResponse, 0, BUFFER_SIZE);
					read(client_socket, serverResponse, BUFFER_SIZE);
					printf("%s", serverResponse);
				} while (strstr(serverResponse, "226 Transfer complete") == NULL);
				//printf("leaving if statement\n");
            }

            else if (status == 1 && strncmp(command, "STOR", 4) == 0) {
                char filename[256];
                sscanf(buffer + 5, "%s", filename);

				memset(buffer, 0, BUFFER_SIZE);//++++
                //read(client_socket, buffer, BUFFER_SIZE); // check 200
                printf("main: %s", buffer);//++++

				struct sockaddr_in serverAddr;
				socklen_t addrSize = sizeof(serverAddr);
				int dataTransferSocket = accept(data_socket, (struct sockaddr *)&serverAddr, &addrSize);
				if (dataTransferSocket < 0) {
					perror("Failed to accept data connection");
					exit(EXIT_FAILURE);
				}
				printf("before send file:%s\n", buffer);
                send_file(dataTransferSocket, filename); // handle send_file
				char serverResponse[BUFFER_SIZE];
                
                do {
					memset(serverResponse, 0, BUFFER_SIZE);
					read(client_socket, serverResponse, BUFFER_SIZE);
					printf("%s", serverResponse);
				} while (strstr(serverResponse, "226 Transfer complete") == NULL);
            }
			else if (strncmp(command, "LIST", 4) == 0) {
                while (1)
				{
					memset(buffer, 0, BUFFER_SIZE);
					int bytesRead = read(data_socket, buffer, BUFFER_SIZE);
					if (bytesRead <= 0)
						break; 
					printf("%.*s", bytesRead, buffer);
				}
				close(data_socket);
				receiveResponse(client_socket);
            }
			//printf("exiting list, retr, stor\n");
		}	
		else
		{
			write(client_socket, "INVALID", strlen("INVALID"));
			receiveResponse(client_socket);
		}
	}
	return (0);
}
