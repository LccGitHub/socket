#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>

#define PORT 1024
#define BUFFER_1K 1024

int main(int argc, char** argv)
{
	int client_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (client_fd < 0) {
		perror("socket failed:");
		exit(1);
	}

	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	if (inet_aton("127.0.0.1", &server_addr.sin_addr) ==0) {
		printf("server ip error \n");
		exit(1);
	}

	if (connect(client_fd, (struct sockaddr*)&server_addr, (socklen_t)sizeof(server_addr)) < 0) {
		perror("connect failed:");
		close(client_fd);
		exit(1);
	}

	printf("connect success ....");

	while(1) {
		char buffer[BUFFER_1K];
		memset(buffer, 0, sizeof(buffer));
		int res = write(client_fd, "hello", strlen("hello"));
		printf("write res = %d \n", res);
		printf("wait for receive ...");
#if 1
		int length = read(client_fd, buffer, sizeof(buffer));
		if (length <= 0) {
			perror("read failed:");
			close(client_fd);
			exit(1);
		}
		printf("read buf:%s\n", buffer);
#endif
	}
	close(client_fd);
	return 0;
}

