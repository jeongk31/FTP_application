#include "client.h"

void send_command(int client_socket, const char *command)
{
	char buffer[BUFFER_SIZE];
	sprintf(buffer, "%s\r\n", command);
	send(client_socket, buffer, strlen(buffer), 0);

	memset(buffer, 0, BUFFER_SIZE);
	read(client_socket, buffer, BUFFER_SIZE);
	printf("%s", buffer);
}

int	countWords(const char *str)
{
	bool	inWord = false;
	int		count = 0;

	while (*str != '\0')
	{
		//urrent character is whitespace
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
		str++; // Move to the next character
	}

	//count+1 if  last character not whitespace (to count last word)
	if (inWord == true)
		count++;
	return (count);
}

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
    serverAddr.sin_port = htons(DATA_PORT);

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

//send PORT
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
				count++; // Increment word count when encountering whitespace after a word
				inWord = false;
				if (count == 2)
				{ // Check if this is the second word
					start = i + 1; // Set start index to the character after whitespace
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