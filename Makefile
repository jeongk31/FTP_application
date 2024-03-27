CNAME		= ftp_client

SNAME		= ftp_server

SRCC_FILE	= client/ftp_client.c client/client_utils.c client/client_dataConnect.c client/client_communicate.c client/clientCommands.c

SRCS_FILE	= server/ftp_server.c server/server_utils.c server/serverCommands.c server/server_dataConnect.c

OBJC		= ${SRCC_FILE:.c=.o}

OBJS		= ${SRCS_FILE:.c=.o}

CC			= gcc

CFLAGS		= -Wall -Wextra -Werror -g

RM			= rm -rf

# Determine the operating system
UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Linux)
	LIBS = -lbsd
endif
ifeq ($(UNAME_S),Darwin)
	LIBS = ""
endif

all: ${CNAME} ${SNAME}

${CNAME}: ${OBJC}
	${CC} ${CFLAGS} ${OBJC} -o ${CNAME}

${SNAME}: ${OBJS}
	${CC} ${CFLAGS} ${OBJS} -o ${SNAME} ${LIBS}

clean:
	${RM} ${OBJC}
	${RM} ${OBJS}

fclean:	clean
	${RM} ${CNAME}
	${RM} ${SNAME}
	
re:	fclean all

.PHONY:	all clean fclean re