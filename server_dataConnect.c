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

/* char	*listFiles(void)
{
	FILE	*list = popen("ls -l", "r"); //system command
	if (!list)
	{
		perror("Server file list");
		exit(EXIT_FAILURE);
	}

	//read list
	char	fileList[BUFFER_SIZE];
	size_t	bytes = fread(fileList, 1, sizeof(fileList), list);
	pclose(list);

	//malloc for list string
	char	*result = (char *)malloc(bytes + 1);
	if (!result) //protection
	{
		perror("LIST: memory allocation");
		exit(EXIT_FAILURE);
	}

	//copy fileList to malloc-ed result
	memcpy(result, fileList, bytes);
	result[bytes] = '\0'; //null termination

	return (result);
}

int	sendData(int dataSocket, const char *data)
{
	int	bytes = write(dataSocket, data, strlen(data));
	if (bytes < 0)
	{
		perror("Server data connection: send data");
		exit(EXIT_FAILURE);
	}
	return (bytes);
} */