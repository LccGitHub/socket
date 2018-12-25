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

int main(void)
{
	int socketFd = -1;
    

	socketFd = socket(AF_INET, SOCK_DGRAM, 0);
	if (socketFd < 0) {
		printf("socket failed, err=%s\n", strerror(errno));
	}
	else {

		int flag = 1;
        setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int));


		struct sockaddr_in addr;
		bzero(&addr, sizeof(struct sockaddr_in));
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
		addr.sin_port = htons(SERVER_PORT);

		
		if (bind(socketFd, (struct sockaddr*)&addr, sizeof(struct sockaddr_in)) < 0) {
			printf("bind failed, err = %s\n", strerror(errno));
		}
		else {
			while (1) {
				char buffer[MAX_BUF_SIZE] = { 0 };
				int addrlen = sizeof(struct sockaddr);
				int res = recvfrom(socketFd, buffer, MAX_BUF_SIZE, 0, (struct sockaddr*)&addr, &addrlen);
				printf("server recv res = %d, msg[%s]\n", res, buffer);
				bzero(buffer, MAX_BUF_SIZE);
				res = sendto(socketFd, "server received", 20, 0, (struct sockaddr*)&addr, sizeof(struct sockaddr_in));
				printf("server send res = %d\n", res);
				bzero(buffer, MAX_BUF_SIZE);
			}
		}

	}
	close(socketFd);

}

