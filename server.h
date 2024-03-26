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

/* server_dataConnect.c */
int		setup_data_connection(char *clientIp, int dataPort);

#endif
