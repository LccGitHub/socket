
/**
 * @file UnixSocketServer.h
 * @brief Declaration file of class UnixSocketServer.
 */

#ifndef UnixSocketServer_H
#define UnixSocketServer_H

#ifndef __cplusplus
#    error ERROR: This file requires C++ compilation(use a .cpp suffix)
#endif


#include <string>

#include "SocketBase.h"
#include "SocketListener.h"

class UnixSocketServer: public SocketBase
{
public:


public:
    UnixSocketServer(const std::string& ip, const uint16_t port);
    ~UnixSocketServer();


    bool send(const char* buf, const uint32_t length);

    bool start();
    bool stop();

private:
    bool initSocket();
    bool onDataAvailable(SocketClient* client);

    class UnixSocketServerListener: public SocketListener 
    {
    public:
         UnixSocketServerListener(UnixSocketServer* mServer, int socketFd);
         ~UnixSocketServerListener();    

    private:    
        // Overridden from SocketListener:
        bool onDataAvailable(SocketClient* client);

    private:
        UnixSocketServer* mServer;    
    };

private:
    uint16_t m_port;
    std::string m_ip;
    UnixSocketServerListener* mListener;
    int32_t m_listenfd;
};


#endif /* UnixSocketServer_H */
/* EOF */
