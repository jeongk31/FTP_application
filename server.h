#ifndef SERVER_H
# define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>


#define PORT 21
#define BUFFER_SIZE 1024
#define MAX_ATTEMPTS 5

/* server_utils.c */
int		countWords(const char *str);
char*	getName(const char *str);
int		validUser(const char* username);
int		validPassword(const char* username, const char* password);

/* serverCommands.c */
bool	handleUser(int fdConnect, char *buffer);
bool	handlePass(int fdConnect, char *buffer, bool checkedUser, char *username);
void	handleQuit(int fdConnect, char *buffer);
void	handlePwd(int fdConnect, char *buffer, char authenticated);
void	handleCwd(int fdConnect, char *buffer, char authenticated);
void	handleInvalid(int fdConnect, char authenticated);
void	handleClientCwd(int fdConnect, char *buffer, bool authenticated);
void	handleClientInfo(int fdConnect, char *buffer, bool authenticated);
void	handleRetr(int fdConnect, char *buffer, bool authenticated, char *clientIp, int dataPort);
void	handleStor(int fdConnect, char *buffer, bool authenticated, char *clientIp, int dataPort);
void	handleList(int fdConnect, char *buffer, bool authenticated, char *clientIp, int dataPort);

/* server_dataConnect.c */
int		setup_data_connection(char *clientIp, int dataPort);

#endif