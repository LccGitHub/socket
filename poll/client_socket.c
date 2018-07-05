#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<string.h>

int main()
{
	struct sockaddr_in server_addr;
	struct sockaddr_in my_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	memset(&my_addr, 0, sizeof(my_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr("116.62.18.190");
	server_addr.sin_port = htons(8000);
	int client_fd = socket(AF_INET, SOCK_STREAM, 0);
	printf("%s,%d-%d\n", __func__, __LINE__, client_fd);
	if (client_fd < 0){
		perror("socket error:\n");
	}
	printf("%s,%d\n", __func__, __LINE__);
	
	if (connect(client_fd, (struct sockaddr*)&server_addr, sizeof(struct sockaddr)) < 0){
		perror("connect error:\n");
	}
	printf("%s,%d\n", __func__, __LINE__);
	printf("have connected to server \n");
	while(1) {
		printf("please input your want to send data\n");
		char sendbuffer[1024] = {0};
		memset(sendbuffer, 0, sizeof(sendbuffer));
		scanf("%s", sendbuffer);
		write(client_fd, sendbuffer, strlen(sendbuffer));
		memset(sendbuffer, 0, sizeof(sendbuffer));
		read(client_fd, sendbuffer, sizeof(sendbuffer) -1);
		printf("client receive data[%s]\n", sendbuffer);
	}
}


