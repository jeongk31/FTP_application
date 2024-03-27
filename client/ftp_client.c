#include "client.h"

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
	printInitialPrompt();

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
			handleClientInfo(client_socket, buffer);

		else if (strncmp(buffer, "!CWD", 4) == 0)
			handleClientCwd(client_socket, buffer);

		else if (strncmp(buffer, "PWD", 3) == 0 || strncmp(buffer, "CWD", 3) == 0)
		{
			write(client_socket, buffer, strlen(buffer));
			receiveResponse(client_socket);
		}

		else if (strncmp(buffer, "LIST", 4) == 0 || strncmp(buffer, "RETR", 4) == 0 || strncmp(buffer, "STOR", 4) == 0)
		{

			data_socket = send_port_command(client_socket);
			char	command[256];
			strncpy(command, buffer, 4);
			command[4] = '\0';
			
			//record status based on what message server sent back
			int status = send_command(client_socket, buffer);

			if (status == 1 && strncmp(command, "RETR", 4) == 0)
				handleRetr(client_socket, data_socket, buffer);


			else if (status == 1 && strncmp(command, "STOR", 4) == 0)
				handleStor(client_socket, data_socket, buffer);

			else if (status == 1 && strncmp(command, "LIST", 4) == 0)
				handleList(client_socket, data_socket, buffer);
		}	
		else
		{
			write(client_socket, "INVALID", strlen("INVALID"));
			receiveResponse(client_socket);
		}
	}
	return (0);
}
