#include "client.h"


// Function to receive a file
void receive_file(int dataSocket, const char *filename) {
    FILE *file = fopen(filename, "wb");
    char buffer[BUFFER_SIZE];
    int bytesReceived;

    while ((bytesReceived = recv(dataSocket, buffer, BUFFER_SIZE, 0)) > 0) {
        fwrite(buffer, 1, bytesReceived, file);
    }

    fclose(file);
    close(dataSocket);
}

// Function to send a file
void send_file(int dataSocket, const char *filename) {
    FILE *file = fopen(filename, "rb");
    char buffer[BUFFER_SIZE];
    int bytesRead;

    while ((bytesRead = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        send(dataSocket, buffer, bytesRead, 0);
    }

    fclose(file);
    close(dataSocket);
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
		else if (strncmp(buffer, "LIST", 4) == 0 || strncmp(buffer, "RETR", 4) == 0 || strncmp(buffer, "STOR", 4) == 0) // add RETR STOR
		{
			// LIST command is handled here
            send_port_command(client_socket); // Prepare data connection
            send_command(client_socket, buffer);

            if (strncmp(buffer, "RETR", 4) == 0) {
                char filename[256];
                sscanf(buffer + 5, "%s", filename);
                                
                memset(buffer, 0, BUFFER_SIZE);
                read(client_socket, buffer, BUFFER_SIZE); // check 200
                printf("%s", buffer);
                
                receive_file(data_socket, filename); // handle receive_file
                
                do {
					/////printf("tmp: %s\n", buffer);
                    memset(buffer, 0, BUFFER_SIZE);
                    read(client_socket, buffer, BUFFER_SIZE);
                    printf("%s", buffer);
					/////printf("\tcompare value: %d\n", strncmp(buffer, "226", 3));
                } while (strncmp(buffer, "226", 3) != 0);
				//////printf("leaving if statement\n");
            }

            else if (strncmp(buffer, "STOR", 4) == 0) {
                char filename[256];
                sscanf(buffer + 5, "%s", filename);
                send_file(data_socket, filename); // handle send_file
            }
			else if (strncmp(buffer, "LIST", 4) == 0) {
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
			////////printf("exiting list, retr, stor\n");
		}	
		else
		{
			write(client_socket, "INVALID", strlen("INVALID"));
			receiveResponse(client_socket);
		}
	}
	return (0);
}