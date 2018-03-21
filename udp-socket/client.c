#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<errno.h>

#define BUFFER_SIZE 1024

int main(int argc, char* argv[])
{
    printf("usage:\n ./a.out ip port \n");
    if (argc != 3) 
    {
        printf("input error, please see usage \n");
        return 0;
    }

    while(1) 
    {
        /* int socket(int domain, int type, int protocol) */
        int client_fd = socket(AF_INET, SOCK_DGRAM, 0);
        if (client_fd < 0) 
        {
			perror("socket failed:");
            exit(1);
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
        socklen_t server_addr_length = sizeof(server_addr);

        while(1) 
        {
            char buffer[BUFFER_SIZE + 1];
            memset(buffer, 0, sizeof(buffer));
			printf("please input send data:\n");
			scanf("%s",buffer);
            sendto(client_fd, buffer, strlen(buffer), 0, (struct sockaddr*)&server_addr, sizeof(server_addr));

            int length = recvfrom(client_fd, buffer, BUFFER_SIZE, 0, NULL, NULL);
            if (length == 0 )
            {
                printf("recv length is zero, peer is shutdown \n");
                break;
            }
			else if (length == -1) {
				perror("recvfrom error:");
				exit(1);
			}
            else 
            {
                printf("receive data : %s \n", buffer);
                bzero(buffer, sizeof(buffer));
            }
        }
        close(client_fd);
    }
    return 0;
}
    
