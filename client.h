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
int	send_command(int client_socket, const char *command);
int		countWords(const char *str);
int send_port_command(int controlSocket);
void	receiveResponse(int client_socket);
char*	getName(const char *str);

#endif