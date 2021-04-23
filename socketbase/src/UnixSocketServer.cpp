/**
 * Copyright @ 2018 - 2020 Suntec Software(Shanghai) Co., Ltd.
 * All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are NOT permitted except as agreed by
 * Suntec Software(Shanghai) Co., Ltd.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 */
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <string.h>
#include <sys/un.h>

#include "UnixSocketServer.h"

#include "SocketLog.h"

UnixSocketServer::UnixSocketServer(const std::string &ip, const uint16_t port)
    : SocketBase(ip, port)
    , m_port(port)
    , m_ip(ip)
    , mListener(NULL)
    , m_listenfd(-1)
{
    initSocket();
}

/*
 * create_socket - creates a Unix domain socket in ANDROID_SOCKET_DIR
 * ("/dev/socket") as dictated in init.rc. This socket is inherited by the
 * daemon. We communicate the file descriptor's value via the environment
 * variable ANDROID_SOCKET_ENV_PREFIX<name> ("ANDROID_SOCKET_foo").
 */
bool UnixSocketServer::initSocket()
{
    struct sockaddr_un addr;
    int ret = -1;
    bool res = true;

    m_listenfd = socket(PF_UNIX, SOCK_STREAM, 0);
    if (m_listenfd < 0) {
        SLOGE("Failed to open socket '%s': %s\n", m_ip.c_str(), strerror(errno));
        res = false;
    }
    else {
        memset(&addr, 0 , sizeof(addr));
        addr.sun_family = AF_UNIX;
        snprintf(addr.sun_path, sizeof(addr.sun_path), "/tmp/%s", m_ip.c_str());

        ret = unlink(addr.sun_path);
        if (ret != 0 && errno != ENOENT) {
            SLOGE("Failed to unlink old socket '%s': %s\n", m_ip.c_str(), strerror(errno));
            close(m_listenfd);
            res = false;
        }
        else {
            ret = bind(m_listenfd, (struct sockaddr *) &addr, sizeof (addr));
            if (ret) {
                SLOGE("Failed to bind socket '%s': %s\n", addr.sun_path, strerror(errno));
                close(m_listenfd);
                unlink(addr.sun_path);
                res = false;
            }
            else {
                SLOGI("Created socket '%s' finish \n", addr.sun_path);
            }
        }
    }

    return res;
}



UnixSocketServer::~UnixSocketServer()
{
    stop(); 
}


bool UnixSocketServer::send(const char *buf, const uint32_t length)
{
    bool res = true;
    if (buf == nullptr) {
        res = false;
    }
    else {
        uint32_t length_tmp = length;
        uint8_t sendBuf[BUFSIZ] = { 0 };
        uint32_t length_net = htonl(length_tmp);
        memcpy(sendBuf, &length_net, sizeof (length_net));

        memcpy(sendBuf+4, buf, length);

       if (mListener != NULL) {
            mListener->sendBroadcast(sendBuf, 4 + length);
       }
       else {

       }
    }

    return res;
}

bool UnixSocketServer::start()
{
    if (mListener == NULL) {
        mListener = new UnixSocketServerListener(this, m_listenfd);
        if (mListener != NULL) {
            mListener->startListener();
        }
        else {
            
        }
    }
    else {

    }

    return true;
}

bool UnixSocketServer::stop()
{

    if (mListener != NULL) {
        mListener->stopListener();
    }
    else {
        
    }
    close(m_listenfd);

    return true;
}

bool UnixSocketServer::onDataAvailable(SocketClient* client)
{
    bool res = true;
    if (client == NULL) {
        res = false;
    }
    else {
        int32_t sockfd = client->getSocket();
        uint32_t retReadSize = 0;
        uint32_t expectReadSize = 0;
        const uint8_t PROTOCOL_HEAD_LENGTH = 4;
        const uint32_t BUFFERSIZ = 1024; // 1k
        char header[PROTOCOL_HEAD_LENGTH] = { 0 };

        while (expectReadSize < PROTOCOL_HEAD_LENGTH) {
            retReadSize = read(sockfd, header + expectReadSize, PROTOCOL_HEAD_LENGTH);
            if (retReadSize != 0) {
                expectReadSize += retReadSize;
            }
            else {
                res = false;
                SLOGE("read failed retReadSize:%d\n", retReadSize);
                return res;
            }
        }

        uint32_t* length_net = (uint32_t*)header;
        uint32_t msgLength = htonl(*length_net);
        // printf("read msg length: %d\n", msgLength);
        expectReadSize = 0;
        retReadSize = -1;
        char msgBuff[BUFFERSIZ] = { 0 };

        while (expectReadSize < msgLength) {
            retReadSize = read(sockfd, msgBuff + expectReadSize, msgLength);
            if (retReadSize != 0) {
                expectReadSize += retReadSize;
            }
            else {
                res = false;
                SLOGE("read failed retReadSize:%d\n", retReadSize);
                return res;
            }
        }
        SLOGD("tcp server recevie msg:\n");
        SLOGD("[%s]\n\n", msgBuff);

    }
    return res;
}

UnixSocketServer::UnixSocketServerListener::UnixSocketServerListener(UnixSocketServer* mServer, int socketFd)
    : SocketListener(socketFd, true)
    , mServer(mServer)
{

}

UnixSocketServer::UnixSocketServerListener::~UnixSocketServerListener()
{

}

bool UnixSocketServer::UnixSocketServerListener::onDataAvailable(SocketClient* client)
{
    bool res = false;
    if (mServer != NULL) {
        res = mServer->onDataAvailable(client);
    }
    else {
        // error
    }
    return res;
}




/* EOF */
