#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<sys/epoll.h>
#include<signal.h>


#define MAX_FD 100
#define MAX_EVENTS 50
#define PORT 1024
#define BUFFER_1K 1024


int epoll_fd;
struct epoll_event events[MAX_EVENTS];

void onAccept(int sock);
void on_recv(int sock);

int main(int argc, char** argv)
{
	signal(SIGPIPE, SIG_IGN);
	int server_fd = socket(PF_INET, SOCK_STREAM, 0);
	if (server_fd < 0) {
		perror("socket failed:");
		exit(1);
	}
	
	int on = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int)) < 0) {
		perror("setsockopt failed:");
	}

	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
		perror("bind faild:");
		close(server_fd);
		exit(1);
	}

	if (listen(server_fd, MAX_FD) < 0) {
		perror("listen failed:");
		close(server_fd);
		exit(1);
	}

	printf("listening ....\n");

	epoll_fd = epoll_create(MAX_EVENTS); //create epoll descriptor
	if (epoll_fd < 0) {
		perror("epoll_create failed:");
		close(server_fd);
		exit(1);
	}

	struct epoll_event event;
	event.events = EPOLLIN | EPOLLET | EPOLLOUT;
	event.data.fd = server_fd;

	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event) < 0) { //registered monitoring events
		perror("epoll_ctl failed:");
		close(server_fd);
		exit(1);
	}

	char recv_buf[BUFFER_1K];
	while(1) {
		int wait_fd = epoll_wait(epoll_fd, events, MAX_EVENTS, 5000);
		if (wait_fd < 0) {
			perror("epoll_wait error:");
			break;
		}
		else if (wait_fd == 0) {
			printf("timeout \n");
			continue;
		}
		else {
			int i = 0;
			for (i = 0; i < wait_fd; i++)
			{
				if ((events[i].events & EPOLLERR) ||
						(events[i].events & EPOLLHUP) ||
						(!(events[i].events & EPOLLIN)))
				{
					printf("epoll error \n");
					close(events[i].data.fd);
					continue;
				}

				if (events[i].data.fd == server_fd)
				{
					onAccept(server_fd);
				}
				else if (events[i].events & EPOLLIN){
					//onRecv(events[i].data.fd);
					int read_fd = events[i].data.fd;
					if (read_fd < 0) {
						continue;
					}
					int n = 0;
					int len;
					len = read(read_fd, recv_buf, sizeof(recv_buf));
					if (len  < 0) {
						perror("read failed:");
						if (errno == ECONNRESET) {
							close(read_fd);
							events[i].data.fd = -1;
						}
						else {
							printf("read erro \n");
						}
					}
					else if (len == 0) {
							close(read_fd);
							events[i].data.fd = -1;
					}
					else {
						recv_buf[len] = 0;
						printf("read:%s, len =%d\n", recv_buf, len);
						memset(recv_buf, 0, sizeof(recv_buf));
						write(read_fd, "OK", strlen("OK"));
						event.data.fd = read_fd;
						event.events = EPOLLOUT | EPOLLET;
						//epoll_ctl(epoll_fd, EPOLL_CTL_ADD, read_fd, &event);
					}
				}
				else if (events[i].events & EPOLLOUT){
					printf("write event!!! \n");
					int write_fd = events[i].data.fd;
					write(write_fd, "OK", 3);
					
					event.data.fd = write_fd;
					event.events = EPOLLOUT | EPOLLET;
					epoll_ctl(epoll_fd, EPOLL_CTL_ADD, write_fd, &event);
				}
			}
		}
	}
	close(epoll_fd);
	close(server_fd);
	return 0;
}


void onAccept(int sock)
{
	int conn_fd = accept(sock, NULL, NULL);
	if (conn_fd < 0) {
		perror("accept faild:");
		return;
	}
	else {
		printf("new connection client[%d]\n", conn_fd);
	}

	/*add new connect fd  to registered monitor list*/
	struct epoll_event event;
	event.data.fd = conn_fd;
	event.events = EPOLLIN | EPOLLET;
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn_fd, &event);

	return;
}

void onRecv(int sock)
{
	char buffer[BUFFER_1K];
	memset(buffer, 0, sizeof(buffer));
	while(1)
	{
		int ret = read(sock, buffer, BUFFER_1K);
		if (ret <= 0) {
			perror("read faild:");
			close(sock);
		}
		else {
			printf("receive:%s \n", buffer);
			struct epoll_event event;
			event.data.fd = sock;
			event.events = EPOLLOUT | EPOLLET;
			epoll_ctl(epoll_fd, EPOLL_CTL_MOD, sock, &event);
		}
	}
	
}

