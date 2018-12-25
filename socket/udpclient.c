#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>


#define SERVER_PORT 8888
#define MAX_BUF_SIZE 1024

int main(int argc, char* argv[])
{
	int socketFd = -1;
    
	if (argc != 2) {
		printf("Usage:%s server_ip\n", argv[0]);
		exit(-1);
	}

	socketFd = socket(AF_INET, SOCK_DGRAM, 0);
	if (socketFd < 0) {
		printf("socket failed, err=%s\n", strerror(errno));
	}
	else {
		struct sockaddr_in addr;
		bzero(&addr, sizeof(struct sockaddr_in));
		addr.sin_family = AF_INET;
		addr.sin_port = htons(SERVER_PORT);

		if (inet_aton(argv[1], &addr.sin_addr) < 0) {
			printf("inet_aton failed, err = %s\n", strerror(errno));
		}
		else {
			while (1) {
				printf("Please input send msg:\n");
				char buffer[MAX_BUF_SIZE] = { 0 };
				fgets(buffer, MAX_BUF_SIZE, stdin);
				int res = sendto(socketFd, buffer, strlen(buffer), 0, (struct sockaddr*)&addr, sizeof(struct sockaddr_in));
				printf("sendto res = %d\n", res);
				bzero(buffer, MAX_BUF_SIZE);
				int addrlen = sizeof(struct sockaddr);
				res = recvfrom(socketFd, buffer, MAX_BUF_SIZE, 0, (struct sockaddr*)&addr, &addrlen);
				printf("client recvfrom res = %d, %s\n", res, buffer);
				bzero(buffer, MAX_BUF_SIZE);
			}
		}

	}
	close(socketFd);

}

