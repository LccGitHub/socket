#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<poll.h>
#include<netinet/in.h>
#include<string.h>
#include<string>
#include<vector>
#include<unistd.h>
#include<pthread.h>
#include<fcntl.h>
#include<json/json.h>
#include<cstring>
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
			close(mFd);
			printf("destory ClientInfo[%d] \n", mFd);
		}
		void matchReadData(char* data)
		{
			printf("receive fd[%d], data[%s]\n", mFd, data);
			/*TO DO==> to match data to do action*/
			std::string buffer(data);
			Json::Reader reader;
			Json::Value root;
			if (reader.parse(buffer, root)) {
				int jsonCnt = root.size();
				printf("json cnt =%d\n", jsonCnt);
					if(!root["ALLDoor"].isNull()){
						std::string out= root["ALLDoor"].asString();
						printf("ALLDoor[%s]\n", out.c_str());
					}
					if (!root["IMEI"].isNull()){
						std::string out= root["IMEI"].asString();
						printf("IMEI[%s]\n", out.c_str());
					}
			}
#if 0
			json_object* jsonObj = json_tokener_parse(data);
			if (jsonObj != NULL) {
				json_object_object_foreach(jsonObj, key, value) {
					const char* valStr = json_object_to_json_string(value);
					if (strncmp(key, "ALLDoor", sizeof("ALLDoor")) == 0) {
						printf("ALLDoor[%s]\n", valStr);
					}
					else if (strncmp(key, "IMEI", sizeof("IMEI")) == 0){
						printf("IMEI[%s]\n", valStr);
					}
					else {
						printf("cant not match \n");
					}
				}
				json_object_put(jsonObj);
			}
			else {
				printf("json_tokener_parse failed \n");
			}
#endif
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
	printf("add Client, current mClientInfo size is %zd \n", mClientInfo.size());
}
void removeClientInfo(ClientInfo* clientInfo)
{
	printf("remove Client, befor mClientInfo size is %zd \n", mClientInfo.size());
	for(auto iter = mClientInfo.begin(); iter < mClientInfo.end();) {
		if (*iter == clientInfo) {
			iter = mClientInfo.erase(iter);
			printf("this client have erase \n");
		}
		else {
			iter++;
		}
	}
	printf("remove Client, after mClientInfo size is %zd \n", mClientInfo.size());
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

class Server
{
	public:
		Server()
		{
			createSocket();
			printf("create Server \n");
		}
		~Server()
		{
			closeServer();
			printf("destory Server \n");
		}
		void setnonblocking(int fd)
		{
			int old_option = fcntl(fd, F_GETFL);
			int new_option = old_option | O_NONBLOCK;
			fcntl(fd, F_SETFL, new_option);
		}
		int createSocket()
		{
			int result = 0;
			struct sockaddr_in server_addr;
			memset(&server_addr, 0, sizeof(server_addr));
			server_addr.sin_family = AF_INET; //set ip protocol
			server_addr.sin_addr.s_addr = INADDR_ANY; //server ip addr--allow connect any ip addr
			server_addr.sin_port = htons(8000);// server port
			int server_fd = socket(AF_INET, SOCK_STREAM, 0);
			if (server_fd < 0) {
				perror("create socket failed:");
				result = -1;
			}
			else if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(struct sockaddr)) < 0){
				perror("bind error:");
				result = -1;
			}
			int opt = 1;
			setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));
			setnonblocking(server_fd);
			mServerFd = server_fd;
			return result;

		}
		void startWork()
		{
			struct sockaddr_in client_addr;
			listen(mServerFd, 20);
			int numfds = 0;
			int maxfd = 0;
			//struct pollfd mPollSet[POLL_SIZE];
			mPollSet[0].fd = mServerFd;
			mPollSet[0].events = POLLIN;
			numfds++;
			while(1){
				printf("wait for client(total [%d])...\n", numfds);
				poll(mPollSet, numfds, -1);
				int index = 0;
				for (index = 0; index < numfds; index++) {
					if (mPollSet[index].revents & POLLIN) {
						if (mPollSet[index].fd == mServerFd) {
							socklen_t client_len =  sizeof(client_addr);
							int client_fd = accept(mServerFd, (struct sockaddr*)&client_addr, &client_len);

							mPollSet[numfds].fd = client_fd;
							mPollSet[numfds].events = POLLIN;
							numfds++;
							ClientInfo* client = new ClientInfo(client_fd);
							addClientInfo(client);
							index++;/*for break for loop and retry poll*/
							printf("add Client on fd %d\n", client_fd);
						}
						else {
							char buffer[1024] = {0};
							memset(buffer,0, sizeof(buffer));
							int res = -1;
							res = read(mPollSet[index].fd, buffer, sizeof(buffer)-1);
							printf("have receive cleint[%d], data[%s], res[%d]\n", mPollSet[index].fd, buffer, res);
							matchDataByFd(mPollSet[index].fd, buffer);
							if (res > 0) {
								write(mPollSet[index].fd, "received!", strlen("received!"));
							}
							else if (res == 0) {
								printf("this client[%d] have close\n", mPollSet[index].fd);
								close(mPollSet[index].fd);
								removeClientInfoByFd(mPollSet[index].fd);
								int i = 0;
								for(i = index; i< numfds-1; i++) {
									//mPollSet[index] = mPollSet[index+1];
									memcpy(&mPollSet[index], &mPollSet[index+1], sizeof(mPollSet));
								}
								numfds--;
								index--;/*reason: array have offset one positon forward*/
							}
							else {
								perror("read error:");
							}
						}
					}

				}
			}
		}
		void closeServer()
		{
			close(mServerFd);
		}
	private:
		int mServerFd;
		struct pollfd mPollSet[POLL_SIZE];
};

void* serverHandler(void* args)
{
	Server* server = new Server();
	server->startWork();
	return NULL;
}
int main()
{
	pthread_t server_thr;
	pthread_create(&server_thr, NULL, serverHandler, NULL);
	pthread_join(server_thr, NULL);

	return 0;
}

