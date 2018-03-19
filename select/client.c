#include<stdio.h>
#include<stdlib.h>
#include<netinet/in.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<errno.h>

#define ERR_EXIT(m) \
    do { \
        perror(m); \
        exit(1); \
    }while(0)


int main(int argc, char *argv[])
{
    if (argc != 3) 
    {
        printf("uage: ./%s ip  port \n", argv[0]);
        exit(1);
    }
    //struct sockaddr_in clientaddr;
    //clientaddr.sin_family = AF_INET;
    //clientaddr.sin_addr.s_addr = htons(INADDR_ANY);
    //clientaddr.sin_port = htons(0);
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd < 0) 
    {
        ERR_EXIT("client create socket failed:");
    }
#if 0
    if (bind(client_fd, (struct sockaddr*)&clientaddr, sizeof(clientaddr)) < 0) 
    {
        ERR_EXIT("client bind failed:");
    }
#endif
    
    struct sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(atoi(argv[2]));

    if (inet_aton(argv[1], &serveraddr.sin_addr) == 0)
    {
        printf("Server Ip address Error!\n");
        exit(1);
    }

    if (connect(client_fd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0) 
    {
        ERR_EXIT("client connect failed:");
    }

    while(1)
    {
        int res = send(client_fd, "hello", 5, 0);
		printf("send result is %d \n", res);
        sleep(10);
    }
    return 0;

}
