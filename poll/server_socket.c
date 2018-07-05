#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<poll.h>
#include<netinet/in.h>
#include<string.h>
#define POLL_SIZE 32
int main()
{
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET; //set ip protocol
	server_addr.sin_addr.s_addr = INADDR_ANY; //server ip addr--allow connect any ip addr
	server_addr.sin_port = htons(8000);// server port
	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0) {
		perror("create socket failed:");
	}
	else if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(struct sockaddr)) < 0){
		perror("bind error:");
	}
	int opt = 0;
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));
	listen(server_fd, 20);
	int numfds = 0;
	int maxfd = 0;
	struct pollfd poll_set[POLL_SIZE];
	poll_set[0].fd = server_fd;
	poll_set[0].events = POLLIN;
	numfds++;
	while(1){
		printf("wait for client(total [%d])...\n", numfds);
		poll(poll_set, numfds, -1);
		int index = 0;
		for (index = 0; index < numfds; index++) {
			if (poll_set[index].revents & POLLIN) {
				if (poll_set[index].fd == server_fd) {
					socklen_t client_len =  sizeof(client_addr);
					int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);

					poll_set[numfds].fd = client_fd;
					poll_set[numfds].events = POLLIN;
					numfds++;
					printf("add Client on fd %d\n", client_fd);
				}
				else {
					char buffer[1024] = {0};
					memset(buffer,0, sizeof(buffer));
					int res = -1;
					res = read(poll_set[index].fd, buffer, sizeof(buffer)-1);
					printf("have receive cleint[%d], data[%s], res[%d]\n", poll_set[index].fd, buffer, res);
					if (res > 0) {
						write(poll_set[index].fd, "received!", strlen("received!"));
					}
					else if (res == 0) {
						printf("this client[%d] have close\n", poll_set[index].fd);
						close(poll_set[index].fd);
#if 0
						poll_set[index].events = 0;
						poll_set[index].fd = -1;
#endif
						int i = 0;
#if 1
						for(i = index; i< numfds-1; i++) {
							//poll_set[index] = poll_set[index+1];
							memcpy(&poll_set[index], &poll_set[index+1], sizeof(poll_set));
						}
						numfds--;
#endif
					}
					else {
						perror("read error:");
					}
				}
			}

		}
	}

	return 0;
}

