#include "server.h"

bool	handleUser(int fdConnect, char *buffer)
{
	bool	checkedUser = false;
	char	*username = NULL;
	if (countWords(buffer) != 2)
		write(fdConnect, "503 Bad sequence of commands.\r\n", strlen("503 Bad sequence of commands.\r\n"));
	else
	{
		//get argument after command
		username = getName(buffer);
		int validity = validUser(username);

		 //file open failure
		if (validity == -1)
			write(fdConnect, "550 No such file or directory.\r\n", strlen("550 No such file or directory.\r\n"));
		
		//invalid username
		else if (username == NULL || validity == 0)
		{
			write(fdConnect, "530 Not logged in.\r\n", strlen("530 Not logged in.\r\n"));
			printf("Incorrect username\n");
		}

		//valid username
		else
		{
			write(fdConnect, "331 Username OK, need password.\r\n", strlen("331 Username OK, need password.\r\n"));
			printf("Successful username verification\n");
			checkedUser = true;
		}
	}
	return (checkedUser);
}

bool	handlePass(int fdConnect, char *buffer, bool checkedUser, char *username)
{
	bool	authenticated = false;
	char	*password = NULL;

	password = getName(buffer);
	if (countWords(buffer) != 2)
		write(fdConnect, "503 Bad sequence of commands.\r\n", strlen("503 Bad sequence of commands.\r\n"));

	//username not verified
	else if (checkedUser == false)
		write(fdConnect, "530 Not logged in.\r\n", strlen("530 Not logged in.\r\n"));

	//username verified
	else
	{
		//get argument after command
		password = getName(buffer);
		int validity = validPassword(username, password);

		//users file open failure
		if (validity == -1)
			write(fdConnect, "550 No such file or directory.\r\n", strlen("550 No such file or directory.\r\n"));
		
		//wrong password
		if (password == NULL || validity == 0)
		{
			write(fdConnect, "530 Not logged in.\r\n", strlen("530 Not logged in.\r\n"));
			printf("Incorrect password\n");
		}

		//valid password
		else
		{
			write(fdConnect, "230 User logged in, proceed.\r\n", strlen("230 User logged in, proceed.\r\n"));
			printf("Successful login\n");
			authenticated = true;
			/* if (username != NULL)
				free(username);
			if (password != NULL)
				free(password); */
		}
	}
	return (authenticated);
}

void	handleQuit(int fdConnect, char *buffer)
{
	//wrong format
	if (countWords(buffer) != 1)
		write(fdConnect, "503 Bad sequence of commands.\r\n", strlen("503 Bad sequence of commands.\r\n"));

	//client quits
	else
	{
		write(fdConnect, "221 Service closing control connection.\r\n", strlen("221 Service closing control connection.\r\n"));
		printf("Closed!\n");
		close(fdConnect);
		exit(EXIT_SUCCESS);
	}
}

void	handlePwd(int fdConnect, char *buffer, char authenticated)
{
	//wrong format
	if (countWords(buffer) != 1)
		write(fdConnect, "503 Bad sequence of commands.\r\n", strlen("503 Bad sequence of commands.\r\n"));

	//user not authenticated
	else if (!authenticated)
		write(fdConnect, "530 Not logged in.\r\n", strlen("530 Not logged in.\r\n"));

	//valid user, handle command
	else
	{
		char	msg[256] = "257 ";
		char	cwd[256];
		
		if (getcwd(cwd, sizeof(cwd)) != NULL)
		{
			strcat(msg, cwd);
			strcat(msg, ".\r\n");
			write(fdConnect, msg, strlen(msg));
			printf("Pritning current directory path of server.\n");
		}
		else
			write(fdConnect, "202 Command not implemented.\r\n", strlen("202 Command not implemented.\r\n"));
	}
}

void	handleCwd(int fdConnect, char *buffer, char authenticated)
{
	//wrong format
	if (countWords(buffer) != 2)
		write(fdConnect, "503 Bad sequence of commands.\r\n", strlen("503 Bad sequence of commands.\r\n"));

	//user not authenticated
	else if (!authenticated)
		write(fdConnect, "530 Not logged in.\r\n", strlen("530 Not logged in.\r\n"));

	//valid user, handle command
	else
	{
		//extract directory path
		char *directory = getName(buffer);

		if (chdir(directory) == 0)
		{
			//add command to change directory if needed

			//print success message to server
			printf("Changing server's directory to: %s\n", directory);

			//send success message to client
			char responseMessage[512]; // Make sure this buffer is large enough to hold the entire message.
			sprintf(responseMessage, "200 directory changed to %s.\r\n", directory);
			write(fdConnect, responseMessage, strlen(responseMessage));
		}
		else
		{
			write(fdConnect, "550 Failed to change directory.\r\n", strlen("550 Failed to change directory.\r\n"));
			printf("550 Failed to change directory\n");
		}
		//free(directory);
	}
}

void	handleInvalid(int fdConnect, char authenticated)
{
	//user not authenticated
	if (!authenticated)
		write(fdConnect, "530 Not logged in.\r\n", strlen("530 Not logged in.\r\n"));

	//handle invalid command
	else
		write(fdConnect, "202 Command not implemented.\r\n", strlen("202 Command not implemented.\r\n"));
}

void	handleClientCwd(int fdConnect, char *buffer, bool authenticated)
{
	//wrong format
	if (countWords(buffer) != 2)
		write(fdConnect, "503 Bad sequence of commands.\r\n", strlen("503 Bad sequence of commands.\r\n"));

	//user not authenticated
	else if (!authenticated)
		write(fdConnect, "530 Not logged in.\r\n", strlen("530 Not logged in.\r\n"));

	//valid user, handle command
	else
	{
		write(fdConnect, "100 Command okay.\r\n", strlen("100 Command okay.\r\n"));
		printf("Changing client's directory\n");
	}
}

void	handleClientInfo(int fdConnect, char *buffer, bool authenticated)
{
	//wrong format
	if (countWords(buffer) != 1)
		write(fdConnect, "503 Bad sequence of commands.\r\n", strlen("503 Bad sequence of commands.\r\n"));

	//user not authenticated
	else if (!authenticated) 
		write(fdConnect, "530 Not logged in.\r\n", strlen("530 Not logged in.\r\n"));

	//valid user, handle command
	else
	{
		write(fdConnect, "100 Command okay.\r\n", strlen("100 Command okay.\r\n"));
		printf("Printing client's information\n");
	}
}

void	handleRetr(int fdConnect, char *buffer, bool authenticated, char *clientIp, int dataPort)
{
	//wrong format
	if (countWords(buffer) != 2)
		write(fdConnect, "503 Bad sequence of commands.\r\n", strlen("503 Bad sequence of commands.\r\n"));

	//user not authenticated
	if (!authenticated)
		write(fdConnect, "530 Not logged in.\r\n", strlen("530 Not logged in.\r\n"));

	//valid user, handle command
	else
	{
		//extract file name
		char filename[256];
		sscanf(buffer + 5, "%s", filename);

		//open file to read
		FILE *file = fopen(filename, "rb");
		if (file == NULL)
		{
			printf("Requested file not found\n");
			write(fdConnect, "550 File not found.\r\n", strlen("550 File not found.\r\n"));
		}
		else
		{
			//message client that data connection will be opened
			write(fdConnect, "150 Opening data connection.\r\n", strlen("150 Opening data connection.\r\n"));

			//set up data connection
			int dataSocket = setup_data_connection(clientIp, dataPort);
			if (dataSocket < 0)
			{
				perror("Failed to setup data connection");
				fclose(file);
			}

			//send file to client
			char fileBuffer[BUFFER_SIZE];
			int bytesRead;
			while ((bytesRead = fread(fileBuffer, 1, BUFFER_SIZE, file)) > 0)
			{
				send(dataSocket, fileBuffer, bytesRead, 0);
			}

			fclose(file);
			close(dataSocket);

			write(fdConnect, "226 Transfer complete.\r\n", strlen("226 Transfer complete.\r\n"));
		}
	}
}

void	handleStor(int fdConnect, char *buffer, bool authenticated, char *clientIp, int dataPort)
{
	//wrong format
	if (countWords(buffer) != 2)
		write(fdConnect, "503 Bad sequence of commands.\r\n", strlen("503 Bad sequence of commands.\r\n"));

	//user not authenticated
	if (!authenticated)
		write(fdConnect, "530 Not logged in.\r\n", strlen("530 Not logged in.\r\n"));

	//valid user, handle command
	else
	{
		//extract file name
		char filename[256];
		int dataSocket = setup_data_connection(clientIp, dataPort);
		sscanf(buffer + 5, "%s", filename);

		FILE *file = fopen(filename, "wb");
		if (file == NULL)
		{
			write(fdConnect, "Could not create file.\r\n", strlen("Could not create file.\r\n"));
		}

		//file opening successful
		else
		{
			write(fdConnect, "150 File status okay; about to open data connection.\r\n", strlen("150 File status okay; about to open data connection.\r\n"));
			printf("Server's file creation okay, beginning data connections\n");

			int bytes;
			char fileBuffer[BUFFER_SIZE];
			if ((bytes = recv(dataSocket, fileBuffer, BUFFER_SIZE, 0)) > 0)
			{
				if (strncmp(fileBuffer, "550", 3) != 0) //file exists on client side
				{
					//printf("file must exist!\n");
					fwrite(fileBuffer, 1, bytes, file);
					while ((bytes = recv(dataSocket, fileBuffer, BUFFER_SIZE, 0)) > 0)
						fwrite(fileBuffer, 1, bytes, file);
					fclose(file);

					write(fdConnect, "226 Transfer complete.\r\n", strlen("226 Transfer complete.\r\n"));
					printf("226 Transfer complete\n");
				}
				else //file doesn't exist on client side
				{
					write(fdConnect, "550 No such file or directory.\r\n", strlen("550 No such file or directory.\r\n"));
					printf("File doesn't exist on client side\n");
					remove(filename);
				}
			}
			close(dataSocket);
		}
	}
}

void	handleList(int fdConnect, char *buffer, bool authenticated, char *clientIp, int dataPort)
{
	//wrong format
	if (countWords(buffer) != 1)
		write(fdConnect, "503 Bad sequence of commands.\r\n", strlen("503 Bad sequence of commands.\r\n"));
	
	//user not authenticated
	if (!authenticated)
		write(fdConnect, "530 Not logged in.\r\n", strlen("530 Not logged in.\r\n"));

	//valid user, handle command
	else
	{
		//set up data connection
		int dataSocket = setup_data_connection(clientIp, dataPort);
		printf("Server's file creation okay, beginning data connections\n");
		printf("Connecting to Client Transfer Socket...\n");

		if (dataSocket < 0)
			printf("Connection to Client Transfer Socket Failed\n");
		else
			printf("Connection Successful\n");

		//message client
		write(fdConnect, "150 File status okay; about to open data connection.\r\n", strlen("150 File status okay; about to open data connection.\r\n"));
		printf("Listing directory\n");

		//execute ls command and save output to a temporary file
		system("ls > temp.txt");

		//open file
		FILE *file = fopen("temp.txt", "r");
		if (file == NULL)
		{
			perror("Server opening temporary file");
			exit(EXIT_FAILURE);
		}

		//read lines of the temporary file, send to client through dataSocket
		char	line[BUFFER_SIZE];
		char	result[BUFFER_SIZE];
		bzero(result, strlen(result));
		while (fgets(line, sizeof(line), file) != NULL)
			strcat(result, line);

		//closing & removing
		fclose(file);
		remove("temp.txt");
		close(dataSocket);

		//send full list to client
		write(fdConnect, result, strlen(result));

		write(fdConnect, "226 Transfer complete.\r\n", strlen("226 Transfer complete.\r\n"));
		printf("226 Transfer complete\n");
	}
}
