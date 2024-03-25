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
#define DATA_PORT 2021


/* client_utils.c */
void	send_command(int client_socket, const char *command);
int		countWords(const char *str);
void send_port_command(int controlSocket);
void	receiveResponse(int client_socket);
char*	getName(const char *str);

/* client_dataConnect.c */
//add data connection related functions

#endif