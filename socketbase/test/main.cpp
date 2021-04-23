
#include <stdio.h>
#include <string.h>

#include "TcpServer.h"
#include "TcpClient.h"

#include "UnixSocketServer.h"
#include "UnixSocketClient.h"

int main(int argc, char* argv[])
{
    if (argc < 2) {
        printf("usage: %s isServer(1 is server, 0 is client)\n", argv[0]);
    }
    else {
        int isServer = atoi(argv[1]);
        SocketBase* sockePtr= NULL;
#ifdef TEST_TCP
        if (isServer) {
            sockePtr = new  TcpServer("127.0.0.1", 8888);
            
        }
        else {
            sockePtr = new  TcpClient("127.0.0.1", 8888);
        }
#else
        if (isServer) {
            sockePtr = new  UnixSocketServer("test", 8888);
            
        }
        else {
            sockePtr = new  UnixSocketClient("test", 8888);
        }
#endif

        if (sockePtr) {
            sockePtr->start();
            bool isLoop = true;
            while(isLoop) {
                printf("please input msg:\n");
                char tmpBufer[1024] = { 0 };
                scanf("%s", tmpBufer);
                if (strncmp(tmpBufer, "et", 2) == 0) {
                    isLoop = false;
                }
                else {
                    sockePtr->send(tmpBufer, strlen(tmpBufer));
                }
            }
            sockePtr->stop();
        }


    }
    return 0;
}