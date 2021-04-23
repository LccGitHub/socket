
/**
 * @file TcpServer.h
 * @brief Declaration file of class TcpServer.
 */

#ifndef TCPSERVER_H
#define TCPSERVER_H

#ifndef __cplusplus
#    error ERROR: This file requires C++ compilation(use a .cpp suffix)
#endif


#include <string>

#include "SocketBase.h"
#include "SocketListener.h"

class TcpServer: public SocketBase
{
public:


public:
    TcpServer(const std::string& ip, const uint16_t port);
    ~TcpServer();


    bool send(const char* buf, const uint32_t length);

    bool start();
    bool stop();

private:
    bool initSocket();
    bool onDataAvailable(SocketClient* client);

    class TcpServerListener: public SocketListener 
    {
    public:
         TcpServerListener(TcpServer* mServer, int socketFd);
         ~TcpServerListener();    

    private:    
        // Overridden from SocketListener:
        bool onDataAvailable(SocketClient* client);

    private:
        TcpServer* mServer;    
    };

private:
    uint16_t m_port;
    std::string m_ip;
    TcpServerListener* mListener;
    int32_t m_listenfd;
};


#endif /* TCPSERVER_H */
/* EOF */
