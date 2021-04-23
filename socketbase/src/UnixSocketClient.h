/**
 * @file UnixSocketClient.h
 * @brief Declaration file of class UnixSocketClient.
 */

#ifndef UNIXSOCKETCLIENT_H
#define UNIXSOCKETCLIENT_H

#ifndef __cplusplus
#    error ERROR: This file requires C++ compilation(use a .cpp suffix)
#endif

#include <string>
#include <thread>

#include "SocketBase.h"
#include "SocketListener.h"



class UnixSocketClient: public SocketBase
{
public:


public:
    UnixSocketClient(const std::string& ip, const uint16_t port);
    ~UnixSocketClient();


    bool start();
    bool stop();

    bool send(const std::string& str, const uint32_t length);
    bool send(const char* buf, const uint32_t length);

private:
    bool initSocket();
    void threadLoop();

    bool onDataAvailable(SocketClient* client);

    class UnixSocketClientListener: public SocketListener 
    {
    public:
         UnixSocketClientListener(UnixSocketClient* mServer, int socketFd);
         ~UnixSocketClientListener();    

    private:    
        // Overridden from SocketListener:
        bool onDataAvailable(SocketClient* client);

    private:
        UnixSocketClient* mServer;    
    };

private:
    bool m_run;
    int m_sockfd;
    uint16_t m_port;
    std::string m_ip;
    std::shared_ptr<std::thread> m_connThread;
    UnixSocketClientListener* mListener;

};


#endif /* UNIXSOCKETCLIENT_H */
/* EOF */
