#include "client.h"

void receive_file(int dataSocket, const char *filename)
{
	printf("Receiving file: %s\n", filename);
	FILE *file = fopen(filename, "wb");
	if (!file)
	{
		perror("Failed to open file on client");
		close(dataSocket);
		return;
	}

	char buffer[BUFFER_SIZE];
	int bytesReceived;

	//read content server is sending
	while ((bytesReceived = recv(dataSocket, buffer, BUFFER_SIZE, 0)) > 0)
		fwrite(buffer, 1, bytesReceived, file);

	if (bytesReceived < 0)
		perror("Receive failed");

	fclose(file);
	close(dataSocket);
	printf("File transfer complete.\n");
}

void send_file(int dataSocket, const char *filename)
{
	FILE *file = fopen(filename, "rb");

	if (!file)
	{
		send(dataSocket, "550 File not found.\r\n", strlen("550 File not found.\r\n"), 0);
		close(dataSocket);
		return ;
	}

	else
	{
		char buffer[BUFFER_SIZE];
		int bytesRead;

		//send contents to server
		while ((bytesRead = fread(buffer, 1, BUFFER_SIZE, file)) > 0)
			send(dataSocket, buffer, bytesRead, 0);

		fclose(file);
		close(dataSocket);
	}
}