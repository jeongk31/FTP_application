#include "client.h"

int	main(void)
{
	struct sockaddr_in	server_address;
	int					client_socket;
	int					data_socket;
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
		else if (strncmp(buffer, "LIST", 4) == 0) // add RETR STOR
		{
			data_socket = send_port_command(client_socket); // Prepare data connection
			send_command(client_socket, buffer);

			if (strncmp(buffer, "LIST", 4) == 0)
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
		}	
		else
		{
			write(client_socket, "INVALID", strlen("INVALID"));
			receiveResponse(client_socket);
		}
	}
	return (0);
}