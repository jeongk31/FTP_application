CNAME		= ftp_client

SNAME		= ftp_server

SRCC_FILE	= ftp_client.c client_utils.c client_dataConnect.c

SRCS_FILE	= ftp_server.c server_utils.c server_dataConnect.c

OBJC		= ${SRCC_FILE:.c=.o}

OBJS		= ${SRCS_FILE:.c=.o}

CC			= gcc

CFLAGS		= -Wall -Wextra -Werror -g

RM			= rm -rf

all: ${CNAME} ${SNAME}

${CNAME}: ${OBJC}
	${CC} ${CFLAGS} ${OBJC} -o ${CNAME}

${SNAME}: ${OBJS}
	${CC} ${CFLAGS} ${OBJS} -o ${SNAME} -lbsd

clean:
	${RM} ${OBJC}
	${RM} ${OBJS}

fclean:	clean
	${RM} ${CNAME}
	${RM} ${SNAME}
	
re:	fclean all

.PHONY:	all clean fclean re