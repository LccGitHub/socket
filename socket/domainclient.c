/**
 * gcc domainserver.c -o server
 * ./client server.socket
 *  "server.socket" is socket server create socket file name
 *
 */



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/un.h>


#define SERVER_PORT 8888
#define MAX_BUF_SIZE 1024

int main(int argc, char* argv[])
{
	int socketFd = -1;
    
	if (argc != 2) {
		printf("Usage:%s server_ip\n", argv[0]);
		exit(-1);
	}

	socketFd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (socketFd < 0) {
		printf("socket failed, err=%s\n", strerror(errno));
	}
	else {
		struct sockaddr_un addr;
		bzero(&addr, sizeof(struct sockaddr_un));
		addr.sun_family = AF_UNIX;
		strcpy(addr.sun_path, "server.socket");

		if (connect(socketFd, (struct sockaddr*)&addr, sizeof(struct sockaddr)) < 0) {
			printf("connect failed, err = %s\n", strerror(errno));
		}
		else {
			while (1) {
				printf("Please input send msg:\n");
				char buffer[MAX_BUF_SIZE] = { 0 };
				fgets(buffer, MAX_BUF_SIZE, stdin);
				int res = write(socketFd, buffer, strlen(buffer));
				printf("sendto res = %d\n", res);
				bzero(buffer, MAX_BUF_SIZE);
				res = read(socketFd, buffer, MAX_BUF_SIZE);
				printf("client recvfrom res = %d, %s\n", res, buffer);
				bzero(buffer, MAX_BUF_SIZE);
			}
		}

	}
	close(socketFd);

}

