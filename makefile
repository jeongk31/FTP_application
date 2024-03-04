all: ftp_server.o ftp_client.o

ftp_server.o: ftp_server.c
	gcc -o ftp_server ftp_server.c

ftp_client.o: ftp_client.c
	gcc -o ftp_client ftp_client.c

clean:
	rm -f ftp_server ftp_client