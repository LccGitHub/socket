#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<signal.h>
#include<errno.h>
#include<string.h>
#include<unistd.h>
#include<sys/wait.h>

#define BUFFER_1K

#define ERR_EXIT(m) \
    do  { \
        perror(m); \
        exit(-1); \
    }while(0)


int main()
{
    signal(SIGPIPE, SIG_IGN);
    int server_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_fd < 0) 
    {
        ERR_EXIT("socket failed:");
    }

    struct sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(5188);
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    int on = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    if(bind(server_fd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0) 
    {
        ERR_EXIT("bind failed:");
    }

    if (listen(server_fd, SOMAXCONN) < 0) 
    {
        ERR_EXIT("listen failed:");
    }

    struct sockaddr_in clientaddr;
    socklen_t clientlen = sizeof(clientaddr);

    int conn;
    int i;
    int client[FD_SETSIZE];
    int maxi = 0;
    for (i = 0; i < FD_SETSIZE; i++ ) 
    {
        client[i] = -1;
    }

    int nready;
    int maxfd = server_fd;

    fd_set rset;
    fd_set allset;
    FD_ZERO(&rset);
    FD_ZERO(&allset);
    FD_SET(server_fd, &allset);

    while(1) 
    {
        rset =  allset;
        nready = select(maxfd + 1, &rset, NULL, NULL, NULL);

        if (nready == -1) 
        {
            if (errno = EINTR) 
            {
                continue;
            }
            ERR_EXIT("listen failed:");
        }

        if (nready == 0) 
            continue;

        if (FD_ISSET(server_fd, &rset)) 
        {
            conn = accept(server_fd, (struct sockaddr*)&clientaddr, &clientlen);
            if (conn == -1) 
            {
                ERR_EXIT("accept failed:");
            }
            for (i = 0; i < FD_SETSIZE; i++) 
            {
                if (client[i] < 0) 
                {
                    client[i] = conn;
					if (i > maxi)
						maxi = i;
					break;
                }
            }
			if (i == FD_SETSIZE) 
			{
				printf("too many clients \n");
                exit(-1);
			}

			printf("receiv connect ip = %s, port = %d, maxi=%d, conn = %d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port), maxi, conn);

			FD_SET(conn, &allset);
			if (conn > maxfd) 
				maxfd = conn;

			if (--nready <= 0)
                continue;
		}

        for (i = 0; i <= maxi; i++) 
        {
            conn = client[i];
            if (conn == -1)
                continue;
            if (FD_ISSET(conn, &rset)) 
            {
                char recvbuf[1024] = {0};
                int ret = read(conn, recvbuf, 1024);
                if (ret == -1) 
                {
                    perror("readline error:");
                }
                else if (ret == 0) 
                {
                    printf("client close \n");
                    FD_CLR(conn, &rset);
                    client[i] = -1;
                    close(conn);
                }
                printf("receive data is [%s] \n", recvbuf);
            }
            write(conn, "data received!", strlen("data received!"));
            if (--nready <= 0)
                continue;
        }
    }
    return 0;
}
