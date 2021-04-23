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

#include "TcpServer.h"

#include "SocketLog.h"

TcpServer::TcpServer(const std::string &ip, const uint16_t port)
    : SocketBase(ip, port)
    , m_port(port)
    , m_ip(ip)
    , mListener(NULL)
    , m_listenfd(-1)
{
    initSocket();
}

bool TcpServer::initSocket()
{
    int opt = 1;
    struct sockaddr_in serv_addr;

    m_listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_listenfd < 0) {
        SLOGE("socket failed, err[%s]\n", strerror(errno));
    }
    else {

        setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        bzero(&serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        // serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        if (inet_pton(AF_INET, m_ip.c_str(), &serv_addr.sin_addr) < 0) {
            SLOGE("inet_pton error for %s err[%s]\n", m_ip.c_str(), strerror(errno));
        }
        serv_addr.sin_port = htons(m_port);

        if (bind(m_listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            SLOGE("socket bind failed, err[%s]\n", strerror(errno));
        }
    }
    return true;
}



TcpServer::~TcpServer()
{
    stop(); 
}


bool TcpServer::send(const char *buf, const uint32_t length)
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

bool TcpServer::start()
{
    if (mListener == NULL) {
        mListener = new TcpServerListener(this, m_listenfd);
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

bool TcpServer::stop()
{

    if (mListener != NULL) {
        mListener->stopListener();
    }
    else {
        
    }
    close(m_listenfd);

    return true;
}

bool TcpServer::onDataAvailable(SocketClient* client)
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

TcpServer::TcpServerListener::TcpServerListener(TcpServer* mServer, int socketFd)
    : SocketListener(socketFd, true)
    , mServer(mServer)
{

}

TcpServer::TcpServerListener::~TcpServerListener()
{

}

bool TcpServer::TcpServerListener::onDataAvailable(SocketClient* client)
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
