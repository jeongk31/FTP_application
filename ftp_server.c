#include "server.h"

int	main(void)
{
	struct sockaddr_in	clientAddr, serverAddr;
	socklen_t			length; //len
	pid_t				childpid;
	char				buffer[BUFFER_SIZE];
	int					fdListen, fdConnect; //listenfd, connfd
	int					fdMax; //nready(change to ready), maxfdp1
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

			printf("Server's Port: %d\n", ntohs(serverAddr.sin_port)); // tmp

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
					printf("Message from tcp client: %s\n", buffer);

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
								continue ;
							}
							else //valid username
							{
								write(fdConnect, "331 Username OK, need password.\r\n", strlen("331 Username OK, need password.\r\n"));
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
								continue;
							}
							else //valid password
							{
								write(fdConnect, "230 User logged in, proceed.\r\n", strlen("230 User logged in, proceed.\r\n"));
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
						else if (!authenticated) // Assuming authenticated is a boolean flag indicating authentication status
						{
							write(fdConnect, "530 Not logged in.\r\n", strlen("530 Not logged in.\r\n"));
							continue ;
						}
						else
						{
							char *directory = getName(buffer);

							if (chdir(directory) == 0)
							{
								// Directory changed successfully
								write(fdConnect, "200 directory changed to ", strlen("200 directory changed to "));
								write(fdConnect, directory, strlen(directory));
								write(fdConnect, ".\r\n", 3);
							}
							else
							{
								write(fdConnect, "550 Failed to change directory.\r\n", strlen("550 Failed to change directory.\r\n"));
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
						else if (!authenticated) // Assuming authenticated is a boolean flag indicating authentication status
						{
							write(fdConnect, "530 Not logged in.\r\n", strlen("530 Not logged in.\r\n"));
							continue ;
						}
						else
						{
							// Acknowledge the !LIST command
							write(fdConnect, "100 Command okay.\r\n", strlen("100 Command okay.\r\n"));
						}
					}
					else if (strncmp(buffer, "!LIST", 5) == 0 || strncmp(buffer, "!PWD", 4) == 0)
					{
						if (countWords(buffer) != 1)
						{
							write(fdConnect, "503 Bad sequence of commands.\r\n", strlen("503 Bad sequence of commands.\r\n"));
							continue;
						}
						else if (!authenticated) // Assuming authenticated is a boolean flag indicating authentication status
						{
							write(fdConnect, "530 Not logged in.\r\n", strlen("530 Not logged in.\r\n"));
							continue ;
						}
						else
							write(fdConnect, "100 Command okay.\r\n", strlen("100 Command okay.\r\n"));
					}
					else if (strncmp(buffer, "PORT", 4) == 0)
					{
						int h1, h2, h3, h4, p1, p2;
            			sscanf(buffer, "PORT %d,%d,%d,%d,%d,%d", &h1, &h2, &h3, &h4, &p1, &p2);
            			dataPort = (p1 * 256) + p2;
            			sprintf(clientIp, "%d.%d.%d.%d", h1, h2, h3, h4);
						write(fdConnect, "200 PORT command successful.\r\n", strlen("200 PORT command successful.\r\n"));
							
					}
					//check for RETR
					// Check for RETR command
        			else if (strncmp(buffer, "RETR", 4) == 0) {
        			    char filename[256];
        			    sscanf(buffer + 5, "%s", filename);

            		// open the file
            		FILE *file = fopen(filename, "rb");
            		if (file == NULL) {
						write(fdConnect, "550 No such file or directory.\r\n", strlen("550 No such file or directory.\r\n"));
            			
            		} else {
            		    write(fdConnect, "150 File status okay; about to open data connection.\r\n", strlen("150 File status okay; about to open data connection.\r\n"));
							

                	int dataSocket = setup_data_connection(clientIp, dataPort);
                	char fileBuffer[BUFFER_SIZE];
                	int bytesRead;

                	// read file and send it over data connection
                	while ((bytesRead = fread(fileBuffer, 1, BUFFER_SIZE, file)) > 0) {
                	    send(dataSocket, fileBuffer, bytesRead, 0);
                	}
                	fclose(file);
                	close(dataSocket);

               		write(fdConnect, "226 Transfer complete.\r\n", strlen("226 Transfer complete.\r\n"));
							
           			}
        		}
					// Check for STOR command
        			else if (strncmp(buffer, "STOR", 4) == 0)
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
							

            			    int bytes;
            			    char fileBuffer[BUFFER_SIZE];
            			    if ((bytes = recv(dataSocket, fileBuffer, BUFFER_SIZE, 0)) > 0) {
            			        fwrite(fileBuffer, 1, bytes, file);
            			    }
            			    fclose(file);
            			    close(dataSocket);

							write(fdConnect, "226 Transfer complete.\r\n", strlen("226 Transfer complete.\r\n"));
							
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

							write(fdConnect, "150 File status okay; about to open data connection.\r\n", strlen("150 File status okay; about to open data connection.\r\n"));
							
							// Execute ls command and save output to a temporary file
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
							printf("result: %s\n", result);

							write(fdConnect, result, strlen(result));

							write(fdConnect, "226 Transfer complete.\r\n", strlen("226 Transfer complete.\r\n"));
							
						}

					}	
					
				}
			}
			close(fdConnect);
		}
	}
	return (0);
}
