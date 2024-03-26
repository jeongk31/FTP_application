#include "server.h"

int	main(void)
{
	struct sockaddr_in	clientAddr, serverAddr;
	socklen_t			length;
	pid_t				childpid;
	char				buffer[BUFFER_SIZE];
	int					fdListen, fdConnect;
	int					fdMax;
	fd_set				readSet;

	char* welcome = "220 Service ready for new user.\r\n";

	//create tcp listening socket
	fdListen = socket(AF_INET, SOCK_STREAM, 0);
	bzero(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	// bind server address to socket
	if (bind(fdListen, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
	{
		perror("bind");
		exit(EXIT_FAILURE);
	}
	if (listen(fdListen, 5) < 0)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}

	//clear descriptor set
	FD_ZERO(&readSet);

	//set max fd
	fdMax = fdListen + 1;

	//loop to run server
	while (1)
	{
		//set fdListen in readySet
		FD_SET(fdListen, &readSet);

		//selec ready descriptor
		select(fdMax, &readSet, NULL, NULL, NULL);

		//accept connection if tcp socket readable
		if (FD_ISSET(fdListen, &readSet))
		{
			length = sizeof(clientAddr);
			fdConnect = accept(fdListen, (struct sockaddr *)&clientAddr, &length);

			printf("Connection established with user %d\n", fdConnect);
            printf("Their port: %d\n", ntohs(clientAddr.sin_port));

			if ((childpid = fork()) == 0)
			{
				close(fdListen);
				write(fdConnect, (const char *)welcome, strlen(welcome));

				char	*username = NULL;
				char	*password = NULL;
				bool	checkedUser = false; //flag to check if client passed 'USER'
				bool	authenticated = false; //flag to check if client passed both 'USER' and 'PASS'
				int		dataPort;
				char	clientIp[256];

				while (1)
				{
					bzero(buffer, sizeof(buffer));
					read(fdConnect, buffer, sizeof(buffer));

					//compare & handle commands
					if (strncmp(buffer, "USER", 4) == 0)
					{
						if (countWords(buffer) != 2)
						{
							write(fdConnect, "503 Bad sequence of commands.\r\n", strlen("503 Bad sequence of commands.\r\n"));
							continue;
						}
						else //correct format
						{
							//get argument after command
							username = getName(buffer);
							int validity = validUser(username);

							if (validity == -1) //file open failure
							{
								write(fdConnect, "550 No such file or directory.\r\n", strlen("550 No such file or directory.\r\n"));
								continue ;
							}
							else if (username == NULL || validity == 0) //invalid username
							{
								write(fdConnect, "530 Not logged in.\r\n", strlen("530 Not logged in.\r\n"));
								printf("Incorrect username\n");
								continue ;
							}
							else //valid username
							{
								write(fdConnect, "331 Username OK, need password.\r\n", strlen("331 Username OK, need password.\r\n"));
								printf("Successful username verification\n");
								checkedUser = true;
							}
						}
					}
					else if (strncmp(buffer, "PASS", 4) == 0)
					{
						if (countWords(buffer) != 2)
						{
							write(fdConnect, "503 Bad sequence of commands.\r\n", strlen("503 Bad sequence of commands.\r\n"));
							continue;
						}
						//username not verified
						else if (checkedUser == false)
						{
							write(fdConnect, "530 Not logged in.\r\n", strlen("530 Not logged in.\r\n"));
							continue ;
						}
						else //username verified
						{
							//get argument after command
							password = getName(buffer);
							int validity = validPassword(username, password);

							if (validity == -1) //file open failure
							{
								write(fdConnect, "550 No such file or directory.\r\n", strlen("550 No such file or directory.\r\n"));
								continue ;
							}
							if (password == NULL || validity == 0)
							{
								write(fdConnect, "530 Not logged in.\r\n", strlen("530 Not logged in.\r\n"));
								printf("Incorrect password\n");
								continue;
							}
							else //valid password
							{
								write(fdConnect, "230 User logged in, proceed.\r\n", strlen("230 User logged in, proceed.\r\n"));
								printf("Successful login\n");
								authenticated = true;
								if (username != NULL)
									free(username);
								if (password != NULL)
									free(password);
							}
						}
					}
					else if (strncmp(buffer, "QUIT", 4) == 0)
					{
						if (countWords(buffer) != 1)
						{
							write(fdConnect, "503 Bad sequence of commands.\r\n", strlen("503 Bad sequence of commands.\r\n"));
							continue;
						}
						else
						{
							write(fdConnect, "221 Service closing control connection.\r\n", strlen("221 Service closing control connection.\r\n"));
							printf("Closed!\n");
							close(fdConnect);
							exit(EXIT_SUCCESS);
						}
					}
					else if (strncmp(buffer, "PWD", 3) == 0)
					{
						if (countWords(buffer) != 1)
						{
							write(fdConnect, "503 Bad sequence of commands.\r\n", strlen("503 Bad sequence of commands.\r\n"));
							continue;
						}
						else if (!authenticated)
						{
							write(fdConnect, "530 Not logged in.\r\n", strlen("530 Not logged in.\r\n"));
							continue ;
						}
						else
						{
							char	msg[256] = "257 ";
							char	cwd[256];
							
							if (getcwd(cwd, sizeof(cwd)) != NULL)
							{
								strcat(msg, cwd);
								strcat(msg, ".\r\n");
								write(fdConnect, msg, strlen(msg));
							}
							else
								write(fdConnect, "202 Command not implemented.\r\n", strlen("202 Command not implemented.\r\n"));
						}
					}
					else if (strncmp(buffer, "CWD", 3) == 0)
					{
						if (countWords(buffer) != 2)
						{
							write(fdConnect, "503 Bad sequence of commands.\r\n", strlen("503 Bad sequence of commands.\r\n"));
							continue;
						}
						else if (!authenticated)
						{
							write(fdConnect, "530 Not logged in.\r\n", strlen("530 Not logged in.\r\n"));
							continue ;
						}
						else
						{
							char *directory = getName(buffer);

							if (chdir(directory) == 0)
							{
								// directory changed successfully
								printf("Changing directory to: %s\n", directory);
								write(fdConnect, "200 directory changed to ", strlen("200 directory changed to "));
								write(fdConnect, directory, strlen(directory));
								write(fdConnect, ".\r\n", 3);
							}
							else
							{
								write(fdConnect, "550 Failed to change directory.\r\n", strlen("550 Failed to change directory.\r\n"));
								printf("550 Failed to change directory\n");
							}

							free(directory);
						}
					}
					else if (strcmp(buffer, "INVALID") == 0)
					{
						if (!authenticated)
						{
							write(fdConnect, "530 Not logged in.\r\n", strlen("530 Not logged in.\r\n"));
							continue ;
						}
						else
							write(fdConnect, "202 Command not implemented.\r\n", strlen("202 Command not implemented.\r\n"));
					}
					else if (strncmp(buffer, "!CWD", 4) == 0)
					{
						if (countWords(buffer) != 2)
						{
							write(fdConnect, "503 Bad sequence of commands.\r\n", strlen("503 Bad sequence of commands.\r\n"));
							continue;
						}
						else if (!authenticated)
						{
							write(fdConnect, "530 Not logged in.\r\n", strlen("530 Not logged in.\r\n"));
							continue ;
						}
						else
						{
							write(fdConnect, "100 Command okay.\r\n", strlen("100 Command okay.\r\n"));
							printf("Changing directory\n");
						}
					}
					else if (strncmp(buffer, "!LIST", 5) == 0 || strncmp(buffer, "!PWD", 4) == 0)
					{
						if (countWords(buffer) != 1)
						{
							write(fdConnect, "503 Bad sequence of commands.\r\n", strlen("503 Bad sequence of commands.\r\n"));
							continue;
						}
						else if (!authenticated) 
						{
							write(fdConnect, "530 Not logged in.\r\n", strlen("530 Not logged in.\r\n"));
							continue ;
						}
						else {
							write(fdConnect, "100 Command okay.\r\n", strlen("100 Command okay.\r\n"));
							printf("Listing directory\n");
						}
					}
					else if (strncmp(buffer, "PORT", 4) == 0)
					{
						int h1, h2, h3, h4, p1, p2;
            			sscanf(buffer, "PORT %d,%d,%d,%d,%d,%d", &h1, &h2, &h3, &h4, &p1, &p2);
            			dataPort = (p1 * 256) + p2;
            			sprintf(clientIp, "%d.%d.%d.%d", h1, h2, h3, h4);
    					printf("Port received: %d,%d,%d,%d,%d,%d\n", h1, h2, h3, h4, p1, p2);
						write(fdConnect, "200 PORT command successful.\r\n", strlen("200 PORT command successful.\r\n"));
							
					}
					//check for RETR
        			else if (strncmp(buffer, "RETR", 4) == 0)
					{
						if (countWords(buffer) != 2)
						{
							write(fdConnect, "503 Bad sequence of commands.\r\n", strlen("503 Bad sequence of commands.\r\n"));
							continue;
						}
						if (!authenticated)
						{
							write(fdConnect, "530 Not logged in.\r\n", strlen("530 Not logged in.\r\n"));
							continue ;
						}
						else
						{
        			    	char filename[256];
							sscanf(buffer + 5, "%s", filename);

							FILE *file = fopen(filename, "rb");
							if (file == NULL) {
								write(fdConnect, "550 File not found.\r\n", 21);
							} else {
								write(fdConnect, "150 Opening data connection.\r\n", 30);

								int dataSocket = setup_data_connection(clientIp, dataPort);
								char fileBuffer[BUFFER_SIZE];
								int bytesRead;

								while ((bytesRead = fread(fileBuffer, 1, BUFFER_SIZE, file)) > 0) {
									send(dataSocket, fileBuffer, bytesRead, 0);
								}

								fclose(file);
								close(dataSocket);
								write(fdConnect, "226 Transfer complete.\r\n", 25);
							}
						}
        			}
					// check for STOR command
        			else if (strncmp(buffer, "STOR", 4) == 0)
					{
						if (countWords(buffer) != 2)
						{
							write(fdConnect, "503 Bad sequence of commands.\r\n", strlen("503 Bad sequence of commands.\r\n"));
							continue;
						}
						if (!authenticated)
						{
							write(fdConnect, "530 Not logged in.\r\n", strlen("530 Not logged in.\r\n"));
							continue ;
						}
						else
						{
        			    	char filename[256];
        			    	int dataSocket = setup_data_connection(clientIp, dataPort);
        			    	sscanf(buffer + 5, "%s", filename);

            				FILE *file = fopen(filename, "wb");
            				if (file == NULL) {
								write(fdConnect, "Could not create file.\r\n", strlen("Could not create file.\r\n"));
            				}
							else {
								write(fdConnect, "150 File status okay; about to open data connection.\r\n", strlen("150 File status okay; about to open data connection.\r\n"));
								printf("File okay, beginning data connections\n");

            				    int bytes;
            				    char fileBuffer[BUFFER_SIZE];
            				    if ((bytes = recv(dataSocket, fileBuffer, BUFFER_SIZE, 0)) > 0) {
            				        fwrite(fileBuffer, 1, bytes, file);
            				    }
            				    fclose(file);
            				    close(dataSocket);

								write(fdConnect, "226 Transfer complete.\r\n", strlen("226 Transfer complete.\r\n"));
								printf("226 Transfer complete\n");

            				}
						}
        			}
					else if(strncmp(buffer, "LIST", 4) == 0)
					{
						if (countWords(buffer) != 1)
						{
							write(fdConnect, "503 Bad sequence of commands.\r\n", strlen("503 Bad sequence of commands.\r\n"));
							continue;
						}
						if (!authenticated)
						{
							write(fdConnect, "530 Not logged in.\r\n", strlen("530 Not logged in.\r\n"));
							continue ;
						}
						else
						{
							//format(2 words) & authentication good
							int dataSocket = setup_data_connection(clientIp, dataPort);
							printf("File okay, beginning data connections\n");
							printf("Connecting to Client Transfer Socket...\n");

							if (dataSocket < 0) {
								printf("Connection to Client Transfer Socket Failed\n");
								continue;
							} else {
								printf("Connection Successful\n");
							}

							write(fdConnect, "150 File status okay; about to open data connection.\r\n", strlen("150 File status okay; about to open data connection.\r\n"));
							printf("Listing directory\n");

							// execute ls command and save output to a temporary file
							system("ls > temp.txt");

							//read lines of the temporary file, send to client through dataSocket
							FILE *file = fopen("temp.txt", "r");
							if (file == NULL)
							{
								perror("Server opening temporary file");
								exit(EXIT_FAILURE);
							}

							char	line[BUFFER_SIZE];
							char	result[BUFFER_SIZE];
							bzero(result, strlen(result));
							while (fgets(line, sizeof(line), file) != NULL)
								strcat(result, line);
								//write(dataSocket, line, strlen(line));
							fclose(file);

							//remove temporary file
							remove("temp.txt");

							//just test
							//write(dataSocket, "000 Test String\r\n", strlen("000 Test String\r\n"));
							//send(dataSocket, "000 Test String\r\n", strlen("000 Test String\r\n"), 0);

							//close data socket
							close(dataSocket);
							// printf("result: %s\n", result);

							write(fdConnect, result, strlen(result));

							write(fdConnect, "226 Transfer complete.\r\n", strlen("226 Transfer complete.\r\n"));
							printf("226 Transfer complete\n");
						}

					}	

				printf("\n");
					
				}
			}
			close(fdConnect);
		}
	}
	return (0);
}
