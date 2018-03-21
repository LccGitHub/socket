#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<errno.h>

#define BUFFER_SIZE 1024
#define LENGTH_OF_LISTEN_QUEUE 20

int main(int argc, char** argv)
{
    printf("usage:\n ./a.out ip port, argc=%d \n", argc);
    if (argc != 3) 
    {
        printf("input error, please see usage \n");
        return 0;
    }


    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    if (inet_aton(argv[1], &server_addr.sin_addr) == 0) 
    {
        printf("Server IP address error, error ip[%s] \n", argv[1]);
        return 0;
    }

    server_addr.sin_port = htons(atoi(argv[2]));

    /* int socket(int domain, int type, int protocol) */
    int server_fd = socket(PF_INET, SOCK_DGRAM, 0);
    if (server_fd < 0) 
    {
        printf("socket failed, fd = %d,%s \n", server_fd, strerror(errno));
        exit(1);
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    socklen_t server_addr_length = sizeof(server_addr);
    if(bind(server_fd, (struct sockaddr*)&server_addr, server_addr_length) < 0) 
    {
        printf("server bind port : %d failed, [%s] \n", server_addr.sin_port, strerror(errno));
        close(server_fd);
        return 0;
    }

    
    while(1) 
    {
        struct sockaddr_in client_addr;
        socklen_t length = sizeof(client_addr);
        char buffer[BUFFER_SIZE + 1];

		ssize_t len = recvfrom(server_fd, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_addr, &length);
		if (len == -1) {
			perror("recvfrom error:");
			exit(1);
		}
		else if (len == 0) {
			printf("peer have shut down \n");
			break;
		}
		else {
			printf("have received data:%s \n", buffer);
		}
		int err = sendto(server_fd, buffer, strlen(buffer), 0, (struct sockaddr*)&client_addr, sizeof(client_addr));
     }
    close(server_fd);
    return 0;
}
    
