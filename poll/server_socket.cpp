#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<poll.h>
#include<netinet/in.h>
#include<string.h>
#include<vector>
#include<unistd.h>
#define POLL_SIZE 32

class ClientInfo
{
	public:
		ClientInfo(int fd)
			: mFd(fd)
		{
			memset(mIMEI, 0, sizeof(mIMEI));
			printf("creat ClientInfo[%d]\n", mFd);
		}
		~ClientInfo()
		{
			printf("destory ClientInfo[%d] \n", mFd);
		}
		void matchReadData(char* data)
		{
			printf("receive fd[%d], data[%s]\n", mFd, data);
			/*TO DO==> to match data to do action*/
		}
		void clearClientInfo()
		{
			mFd = -1;
			memset(mIMEI, 0, sizeof(mIMEI));
		}
		int getClientFd()
		{
			return mFd;
		}
	private:
		int mFd;
		char mIMEI[20];
};

std::vector<ClientInfo*> mClientInfo;

void addClientInfo(ClientInfo* clientInfo)
{
	int i = 0;
	for(i = 0; i < mClientInfo.size();  i++){
		if (mClientInfo[i] == clientInfo){
			printf("this client[%p] have exist \n", clientInfo);
		}
	}
	if (i == mClientInfo.size()) {
		mClientInfo.push_back(clientInfo);
		printf("this client[%p] have add \n", clientInfo);
	}
	printf("add Client, current mClientInfo size is %d \n", mClientInfo.size());
}
void removeClientInfo(ClientInfo* clientInfo)
{
	for(auto iter = mClientInfo.begin(); iter <= mClientInfo.end();) {
		if (*iter == clientInfo) {
			iter = mClientInfo.erase(iter);
		}
		else {
			iter++;
		}
	}
	printf("remove Client, current mClientInfo size is %d \n", mClientInfo.size());
}
ClientInfo* findClientByFd(int fd)
{
	ClientInfo* result = NULL;
	if (fd < 0) {
		printf("this is error fd \n");
	}
	else {
		int i = 0;
		for(i = 0; i < mClientInfo.size();  i++){
			if (mClientInfo[i]->getClientFd() == fd){
				printf("find client[%p] \n", mClientInfo[i]);
				result = mClientInfo[i];
			}
		}
	}
	return result;
}
void addClientInfoByFd(int fd)
{
	ClientInfo* client = findClientByFd(fd);
	addClientInfo(client);
}
void removeClientInfoByFd(int fd)
{
	ClientInfo* client = findClientByFd(fd);
	removeClientInfo(client);
}

void matchDataByFd(int fd, char* data)
{
	ClientInfo* client = findClientByFd(fd);
	client->matchReadData(data);
}
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
					ClientInfo* client = new ClientInfo(client_fd);
					addClientInfo(client);
					printf("add Client on fd %d\n", client_fd);
				}
				else {
					char buffer[1024] = {0};
					memset(buffer,0, sizeof(buffer));
					int res = -1;
					res = read(poll_set[index].fd, buffer, sizeof(buffer)-1);
					printf("have receive cleint[%d], data[%s], res[%d]\n", poll_set[index].fd, buffer, res);
					matchDataByFd(poll_set[index].fd, buffer);
					if (res > 0) {
						write(poll_set[index].fd, "received!", strlen("received!"));
					}
					else if (res == 0) {
						printf("this client[%d] have close\n", poll_set[index].fd);
						close(poll_set[index].fd);
						removeClientInfoByFd(poll_set[index].fd);
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

