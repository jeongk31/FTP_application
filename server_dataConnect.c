#include "server.h"

int setup_data_connection(char *clientIp, int dataPort)
{
	int dataSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (dataSocket < 0)
	{
		perror("Data socket");
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in dataAddr;
	memset(&dataAddr, 0, sizeof(dataAddr));
	dataAddr.sin_family = AF_INET;
	dataAddr.sin_port = htons(dataPort);

	if (inet_pton(AF_INET, clientIp, &dataAddr.sin_addr) <= 0)
	{
		perror("Invalid address");
		exit(EXIT_FAILURE);
	}

	if (connect(dataSocket, (struct sockaddr *)&dataAddr, sizeof(dataAddr)) < 0)
	{
		perror("Data port");
		exit(EXIT_FAILURE);
	}

	return (dataSocket);
}
