#ifndef CLIENT_H
# define CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>


#define PORT 21
#define BUFFER_SIZE 1024


/* client_utils.c */
int		countWords(const char *str);
char*	getName(const char *str);
void	printInitialPrompt(void);

/* client_communicate.c */
int		send_command(int client_socket, const char *command);
void	receiveResponse(int client_socket);
int		send_port_command(int controlSocket);

/* client_dataConnect.c */
void	receive_file(int dataSocket, const char *filename);
void	send_file(int dataSocket, const char *filename);


/* clientCommands.c */
void	handleClientInfo(int client_socket, char *buffer);
void	handleClientCwd(int client_socket, char *buffer);
void	handleRetr(int client_socket, int data_socket, char *buffer);
void	handleStor(int client_socket, int data_socket, char *buffer);
void	handleList(int client_socket, int data_socket, char *buffer);


#endif