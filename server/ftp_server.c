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

			if ((childpid = fork()) == 0)
			{
				close(fdListen);
				write(fdConnect, (const char *)welcome, strlen(welcome));

				char	*username = NULL;
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
						if (handleUser(fdConnect, buffer) == true)
						{
							checkedUser = true;
							username = getName(buffer);
						}
					}
					else if (strncmp(buffer, "PASS", 4) == 0)
						authenticated = handlePass(fdConnect, buffer, checkedUser, username);

					else if (strncmp(buffer, "QUIT", 4) == 0)
						handleQuit(fdConnect, buffer);

					else if (strncmp(buffer, "PWD", 3) == 0)
						handlePwd(fdConnect, buffer, authenticated);

					else if (strncmp(buffer, "CWD", 3) == 0)
						handleCwd(fdConnect, buffer, authenticated);

					else if (strcmp(buffer, "INVALID") == 0)
						handleInvalid(fdConnect, authenticated);

					else if (strncmp(buffer, "!CWD", 4) == 0)
						handleClientCwd(fdConnect, buffer, authenticated);

					else if (strncmp(buffer, "!LIST", 5) == 0 || strncmp(buffer, "!PWD", 4) == 0)
						handleClientInfo(fdConnect, buffer, authenticated);

					else if (strncmp(buffer, "PORT", 4) == 0)
					{
						int h1, h2, h3, h4, p1, p2;
						sscanf(buffer, "PORT %d,%d,%d,%d,%d,%d", &h1, &h2, &h3, &h4, &p1, &p2);
						dataPort = (p1 * 256) + p2;
						sprintf(clientIp, "%d.%d.%d.%d", h1, h2, h3, h4);
						printf("Port received: %d,%d,%d,%d,%d,%d\n", h1, h2, h3, h4, p1, p2);
						write(fdConnect, "200 PORT command successful.\r\n", strlen("200 PORT command successful.\r\n"));	
					}

					else if (strncmp(buffer, "RETR", 4) == 0)
						handleRetr(fdConnect, buffer, authenticated, clientIp, dataPort);

					else if (strncmp(buffer, "STOR", 4) == 0)
						handleStor(fdConnect, buffer, authenticated, clientIp, dataPort);

					else if(strncmp(buffer, "LIST", 4) == 0)
						handleList(fdConnect, buffer, authenticated, clientIp, dataPort);
				}
			}
			close(fdConnect);
		}
	}
	return (0);
}
