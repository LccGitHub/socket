/**
 * @file TcpClient.h
 * @brief Declaration file of class TcpClient.
 */

#ifndef TCPSOCKETCLIENT_H
#define TCPSOCKETCLIENT_H

#ifndef __cplusplus
#    error ERROR: This file requires C++ compilation(use a .cpp suffix)
#endif

#include <string>
#include <thread>

#include "SocketBase.h"
#include "SocketListener.h"



class TcpClient: public SocketBase
{
public:


public:
    TcpClient(const std::string& ip, const uint16_t port);
    ~TcpClient();


    bool start();
    bool stop();

    bool send(const std::string& str, const uint32_t length);
    bool send(const char* buf, const uint32_t length);

private:
    bool initSocket();
    void threadLoop();

    bool onDataAvailable(SocketClient* client);

    class TcpClientListener: public SocketListener 
    {
    public:
         TcpClientListener(TcpClient* mServer, int socketFd);
         ~TcpClientListener();    

    private:    
        // Overridden from SocketListener:
        bool onDataAvailable(SocketClient* client);

    private:
        TcpClient* mServer;    
    };

private:
    bool m_run;
    int m_sockfd;
    uint16_t m_port;
    std::string m_ip;
    std::shared_ptr<std::thread> m_connThread;
    TcpClientListener* mListener;

};


#endif /* TCPSOCKETCLIENT_H */
/* EOF */
