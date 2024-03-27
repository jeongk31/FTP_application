#include "client.h"

void	handleClientInfo(int client_socket, char *buffer)
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

void	handleClientCwd(int client_socket, char *buffer)
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
			printf("200 directory changed to %s.\n", directory);
		}
		else
			printf("550 Failed to change directory.\n");
		free(directory);
	}
	else
		printf("%s", response);
}

void	handleRetr(int client_socket, int data_socket, char *buffer)
{
	char filename[256];
	sscanf(buffer + 5, "%s", filename);
	
	memset(buffer, 0, BUFFER_SIZE);

	struct sockaddr_in serverAddr;
	socklen_t addrSize = sizeof(serverAddr);
	int dataTransferSocket = accept(data_socket, (struct sockaddr *)&serverAddr, &addrSize);
	if (dataTransferSocket < 0) {
		perror("Failed to accept data connection");
		exit(EXIT_FAILURE);
	}
	receive_file(dataTransferSocket, filename); // handle receive_file

	char serverResponse[BUFFER_SIZE];

	do {
		memset(serverResponse, 0, BUFFER_SIZE);
		read(client_socket, serverResponse, BUFFER_SIZE);
		printf("%s", serverResponse);
	} while (strstr(serverResponse, "226 Transfer complete") == NULL && strstr(serverResponse, "503 Bad sequence of commands") == NULL);
}

void	handleStor(int client_socket, int data_socket, char *buffer)
{
	char filename[256];
	sscanf(buffer + 5, "%s", filename);
	memset(buffer, 0, BUFFER_SIZE);

	struct sockaddr_in serverAddr;
	socklen_t addrSize = sizeof(serverAddr);
	int dataTransferSocket = accept(data_socket, (struct sockaddr *)&serverAddr, &addrSize);
	if (dataTransferSocket < 0) {
		perror("Failed to accept data connection");
		exit(EXIT_FAILURE);
	}

	// handle send_file
	send_file(dataTransferSocket, filename);

	//get server's response
	char serverResponse[BUFFER_SIZE];
	do {
		memset(serverResponse, 0, BUFFER_SIZE);
		read(client_socket, serverResponse, BUFFER_SIZE);
		printf("%s", serverResponse);
	} while (strstr(serverResponse, "226 Transfer complete") == NULL && strstr(serverResponse, "550 No such file or directory") == NULL);
}

void	handleList(int client_socket, int data_socket, char *buffer)
{
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