#include "client.h"

int send_command(int client_socket, const char *command)
{
	char buffer[BUFFER_SIZE];
	sprintf(buffer, "%s\r\n", command);
	send(client_socket, buffer, strlen(buffer), 0);

	memset(buffer, 0, BUFFER_SIZE);
	read(client_socket, buffer, BUFFER_SIZE);
	printf("%s", buffer);

	//if commands received non-approving message from server, return status as 1
	if (strncmp(buffer, "550", 3) == 0 || strncmp(buffer, "503", 3) == 0 || strncmp(buffer, "530", 3) == 0 )
		return (0);
	return (1);
}

void receiveResponse(int client_socket)
{
	char buffer[BUFFER_SIZE];
	int readValue;
	bool receivedResponse = false;

	while (!receivedResponse)
	{
		//read server response
		memset(buffer, 0, BUFFER_SIZE);
		readValue = read(client_socket, buffer, BUFFER_SIZE);
		if (readValue <= 0)
		{
			if (errno != EWOULDBLOCK && errno != EAGAIN)
			{
				perror("read failed");
				exit(EXIT_FAILURE);
			}
			//readValue <= 0 & not due to non-blocking socket: server sent nothing yet
			continue; //continue loop & wait for server response
		}
		printf("%s", buffer);
		receivedResponse = true; //response received
	}
}


static int next_data_port = -1;

int send_port_command(int controlSocket)
{
	if (next_data_port == -1)
	{
		struct sockaddr_in localAddress;
		socklen_t addressLength = sizeof(localAddress);
		getsockname(controlSocket, (struct sockaddr *)&localAddress, &addressLength);
		next_data_port = ntohs(localAddress.sin_port) + 1; // dynamic port allocation
	}

	int dataSocket;
	struct sockaddr_in dataAddr;
	bool isBound = false;
	while (!isBound)
	{
		dataSocket = socket(AF_INET, SOCK_STREAM, 0);
		memset(&dataAddr, 0, sizeof(dataAddr));
		dataAddr.sin_family = AF_INET;
		dataAddr.sin_addr.s_addr = htonl(INADDR_ANY);
		dataAddr.sin_port = htons(next_data_port);

		if (bind(dataSocket, (struct sockaddr *)&dataAddr, sizeof(dataAddr)) == 0)
			isBound = true;

		else 
		{
			next_data_port++; // try the next port if bind fails
			close(dataSocket);
		}
	}

	if (listen(dataSocket, 1) < 0)
	{
		perror("listen failed in send_port_command");
		close(dataSocket);
		exit(EXIT_FAILURE);
	}

	unsigned char p1 = next_data_port / 256;
	unsigned char p2 = next_data_port % 256;

	char command[255];
	sprintf(command, "PORT 127,0,0,1,%d,%d", p1, p2);
	send(controlSocket, command, strlen(command), 0);
	receiveResponse(controlSocket);
	next_data_port++; // prep for the next data connection
	return (dataSocket);
}