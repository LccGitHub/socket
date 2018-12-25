
/**
 * gcc domainserver.c -o server -lpthread
 * ./server
 * will create socket file "server.socket"
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


	socketFd = socket(AF_UNIX, SOCK_STREAM, 0); // AF_NET:domain, SOCK_STREAM:tcp
	if (socketFd < 0) {
		printf("socket failed, err=%s\n", strerror(errno));
	}
	else {

		int flag = 1;
		setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int));


		struct sockaddr_un addr;
		bzero(&addr, sizeof(struct sockaddr_un));
		addr.sun_family = AF_UNIX;
		strcpy(addr.sun_path, "server.socket");

		//int len = offsetof(struct sockaddr_un, sun_path) + strlen(addr.sun_path);  
		unlink("server.socket"); 

		if (bind(socketFd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
			printf("bind failed, err = %s\n", strerror(errno));
		}
		else if (listen(socketFd, 5) < 0) {
			printf("listen err = %s\n", strerror(errno));
		}
		else {
			while(1) {
				struct sockaddr_un clientAddr;
				int sinLen = sizeof(struct sockaddr_un);
				int newFd = accept(socketFd, (struct sockaddr*)&clientAddr, &sinLen);
				pthread_t cThread;
				pthread_create(&cThread, NULL, &clientThread, &newFd);
			}
		}

	}
	close(socketFd);

}

