#include "client.h"

int send_command(int client_socket, const char *command)
{
	char buffer[BUFFER_SIZE];
	sprintf(buffer, "%s\r\n", command);
	send(client_socket, buffer, strlen(buffer), 0);

	memset(buffer, 0, BUFFER_SIZE);
	read(client_socket, buffer, BUFFER_SIZE);
	printf("%s", buffer);

	if (strncmp(buffer, "550", 3) == 0 || strncmp(buffer, "503", 3) == 0  || strncmp(buffer, "530", 3) == 0 ) //no file
		return (0);
	return (1);
}

int	countWords(const char *str)
{
	bool	inWord = false;
	int		count = 0;

	while (*str != '\0')
	{
		//current character is whitespace
		if (isspace(*str))
		{
			//previously inside a word, increment word count and reset flag
			if (inWord == true)
			{
				count++;
				inWord = false;
			}
		}
		else //current character not whitespace
			inWord = true;
		str++; // move to the next character
	}

	//count+1 if  last character not whitespace (to count last word)
	if (inWord == true)
		count++;
	return (count);
}


// connect to server
// void connect_to_server(const char *server_address, int *client_socket) {
//     struct sockaddr_in serverAddr;

//     // Create socket
//     *client_socket = socket(AF_INET, SOCK_STREAM, 0);
//     if (*client_socket < 0) {
//         perror("Socket creation failed");
//         exit(EXIT_FAILURE);
//     }

//     // connect to server
//     serverAddr.sin_family = AF_INET;
//     serverAddr.sin_port = htons(DATA_PORT);

//     if (inet_pton(AF_INET, server_address, &serverAddr.sin_addr) <= 0) {
//         perror("Invalid address/ Address not supported");
//         exit(EXIT_FAILURE);
//     }

//     if (connect(*client_socket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
//         perror("Connection Failed");
//         exit(EXIT_FAILURE);
//     }

//     char buffer[BUFFER_SIZE];
//     memset(buffer, 0, BUFFER_SIZE);
//     read(*client_socket, buffer, BUFFER_SIZE);
//     printf("%s", buffer);
// }

static int next_data_port = -1;

//send PORT
int send_port_command(int controlSocket) {
    if (next_data_port == -1) {
        struct sockaddr_in localAddress;
        socklen_t addressLength = sizeof(localAddress);
        getsockname(controlSocket, (struct sockaddr *)&localAddress, &addressLength);
        next_data_port = ntohs(localAddress.sin_port) + 1; // dynamic port allocation
    }

    int dataSocket;
    struct sockaddr_in dataAddr;
    bool isBound = false;
    while (!isBound) {
        dataSocket = socket(AF_INET, SOCK_STREAM, 0);
        memset(&dataAddr, 0, sizeof(dataAddr));
        dataAddr.sin_family = AF_INET;
        dataAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        dataAddr.sin_port = htons(next_data_port);

        if (bind(dataSocket, (struct sockaddr *)&dataAddr, sizeof(dataAddr)) == 0) {
            isBound = true;
        } else {
            next_data_port++; // try the next port if bind fails
            close(dataSocket);
        }
    }

    if (listen(dataSocket, 1) < 0) {
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


char* getName(const char *str)
{
	bool	inWord = false;
	char	*word = NULL;
	int		count = 0;
	int		start = -1;
	int		end = -1;

	for (int i = 0; str[i] != '\0'; i++)
	{
		if (isspace(str[i]))
		{
			if (inWord)
			{
				count++;
				inWord = false;
				if (count == 2)
				{
					start = i + 1; // set start index to  character after whitespace
					inWord = true;
				}
			}
		}
		else
		{
			if (!inWord)
			{
				inWord = true;
				if (count == 1)//first word
					start = i;
			}
		}

		if (count == 2 && !inWord) { // If the second word ends
			end = i; // Set end index
			break;
		}
	}

	//word until end of string
	if (end == -1)
		end = strlen(str);
	
	int length = end - start;

	word = (char *)malloc(sizeof(char) * (length + 1));

	//copy word to malloc-ed word
	strncpy(word, str + start, length);
	word[length] = '\0';

	return (word);
}
