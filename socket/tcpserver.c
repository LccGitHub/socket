
/**
 * gcc tcpserver.c -lptread -o server
 * ./server
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <pthread.h>


#define SERVER_PORT 8888
#define MAX_BUF_SIZE 1024

void* clientThread(void* arg)
{
	int newFd = *((int*)arg);
	printf("clientThread start, newFd = %d \n", newFd);
	while (1) {
		char buffer[MAX_BUF_SIZE] = { 0 };
		int addrlen = sizeof(struct sockaddr);
		int res = read(newFd, buffer, MAX_BUF_SIZE);
		printf("server recv res = %d, msg[%s]\n", res, buffer);
		bzero(buffer, MAX_BUF_SIZE);
		res = write(newFd, "server received", 20);
		printf("server send res = %d\n", res);
		bzero(buffer, MAX_BUF_SIZE);
	}
	return NULL;
}


int main(void)
{
	int socketFd = -1;


	socketFd = socket(AF_INET, SOCK_STREAM, 0); // AF_NET:ipv4, SOCK_STREAM:tcp
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
		// addr.sin_addr.s_addr = inet_addr("192.168.0.1"); 
		addr.sin_port = htons(SERVER_PORT);


		if (bind(socketFd, (struct sockaddr*)&addr, sizeof(struct sockaddr_in)) < 0) {
			printf("bind failed, err = %s\n", strerror(errno));
		}
		else if (listen(socketFd, 5) < 0) {
			printf("listen err = %s\n", strerror(errno));
		}
		else {
			while(1) {
				struct sockaddr_in clientAddr;
				int sinLen = sizeof(struct sockaddr_in);
				int newFd = accept(socketFd, (struct sockaddr*)&clientAddr, &sinLen);
				pthread_t cThread;
				pthread_create(&cThread, NULL, &clientThread, &newFd);
			}
		}

	}
	close(socketFd);

}

