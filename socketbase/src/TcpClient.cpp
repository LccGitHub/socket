/**

 */

#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>

#include "SocketLog.h"
#include "TcpClient.h"


static const uint8_t PROTOCOL_HEAD_LENGTH = 4;

TcpClient::TcpClient(const std::string &ip, const uint16_t port)
    : SocketBase(ip, port)
    , m_run(false)
    , m_sockfd(-1)
    , m_port(port)
    , m_ip(ip)
    , m_connThread(nullptr)
{

}

TcpClient::~TcpClient()
{

}

bool TcpClient::initSocket()
{
    int res = false;
    struct sockaddr_in servaddr;
    m_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_sockfd < 0) {
        perror("TcpClient error");
    }
    else {

        bzero(&servaddr, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(m_port);
        if (inet_pton(AF_INET, m_ip.c_str(), &servaddr.sin_addr) < 0) {
            SLOGE("inet_pton error for %s\n", m_ip.c_str());
        }

        auto ret = connect(m_sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
        if (ret < 0) {
            perror("connect error");
            close(m_sockfd);
            m_sockfd = -1;
        }
        else {
            res = true;
            SLOGI("connect success \n");
        }
    }
    return  res;
}

bool TcpClient::start()
{
    if (m_run) {
        return true;
    }
    else {
        m_run = true;
    }
    m_connThread = std::make_shared<std::thread>(&TcpClient::threadLoop, this);

    return true;
}

bool TcpClient::stop()
{
    m_run = false;

    if (mListener != NULL) {
        mListener->stopListener();
    }
    else {
        
    }
    close(m_sockfd);

    m_connThread->join();


    return true;
}


bool TcpClient::send(const char *buf, const uint32_t length)
{
    if (buf == nullptr) {
        return false;
    }

    uint32_t length_tmp = length;
    uint8_t sendBuf[BUFSIZ] = { 0 };
    uint32_t length_net = htonl(length_tmp);
    memcpy(sendBuf, &length_net, sizeof (length_net));

    memcpy(sendBuf+4, buf, length);
    // printf("tcp send: ");
    // for (uint32_t i = 0; i < 4 + length; ++i) {
    //     printf("%02x ", sendBuf[i]);
    // }
    // printf("\n");

    auto ret = write(m_sockfd, sendBuf, 4 + length);
    if (ret == -1) {
        SLOGE("write error!!! \n");
        return false;
    }
    else {
        return true;
    }
}

void TcpClient::threadLoop()
{
    while (m_run) {
        if (m_sockfd == -1 && initSocket()) {
            if (mListener == NULL) {
                mListener = new TcpClientListener(this, m_sockfd);
                mListener->startListener();
            }
            else if (m_sockfd > 0){
                delete mListener;
                mListener = new TcpClientListener(this, m_sockfd);
                mListener->startListener();
                SLOGE("mListener is not NULL\n");
            }
            // break;
        }
        else {
            sleep(1); // retry connect
        }
    }
    SLOGI("connect thread loop exit[%d]\n", m_run);
}

bool TcpClient::onDataAvailable(SocketClient* client)
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
                close(m_sockfd);
                m_sockfd = -1;
                if (mListener != NULL) {
                    mListener->stopListener();
                    // delete mListener;
                    // mListener = NULL;
                }
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
        SLOGD("tcp client recevie msg:\n");
        SLOGD("[%s]\n\n", msgBuff);

    }
    return res;
}

TcpClient::TcpClientListener::TcpClientListener(TcpClient* mServer, int socketFd)
    : SocketListener(socketFd, false)
    , mServer(mServer)
{

}

TcpClient::TcpClientListener::~TcpClientListener()
{

}

bool TcpClient::TcpClientListener::onDataAvailable(SocketClient* client)
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
