/**

 */

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>

#include "SocketLog.h"
#include "UnixSocketClient.h"


static const uint8_t PROTOCOL_HEAD_LENGTH = 4;

UnixSocketClient::UnixSocketClient(const std::string &ip, const uint16_t port)
    : SocketBase(ip, port)
    , m_run(false)
    , m_sockfd(-1)
    , m_port(port)
    , m_ip(ip)
    , m_connThread(nullptr)
{

}

UnixSocketClient::~UnixSocketClient()
{

}

bool UnixSocketClient::initSocket()
{
    bool res = false;
    struct sockaddr_un addr;
    m_sockfd = socket(PF_UNIX, SOCK_STREAM, 0);
    if (m_sockfd < 0) {
        perror("UnixSocketClient error");
    }
    else {

        memset(&addr, 0 , sizeof(addr));
        addr.sun_family = AF_UNIX;
        snprintf(addr.sun_path, sizeof(addr.sun_path), "/tmp/%s", m_ip.c_str());

        auto ret = connect(m_sockfd, (struct sockaddr*)&addr, sizeof(addr));
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

bool UnixSocketClient::start()
{
    if (m_run) {
        return true;
    }
    else {
        m_run = true;
    }
    m_connThread = std::make_shared<std::thread>(&UnixSocketClient::threadLoop, this);

    return true;
}

bool UnixSocketClient::stop()
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


bool UnixSocketClient::send(const char *buf, const uint32_t length)
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

void UnixSocketClient::threadLoop()
{
    while (m_run) {
        if (m_sockfd == -1 && initSocket()) {
            if (mListener == NULL) {
                mListener = new UnixSocketClientListener(this, m_sockfd);
                mListener->startListener();
            }
            else if (m_sockfd > 0){
                delete mListener;
                mListener = new UnixSocketClientListener(this, m_sockfd);
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

bool UnixSocketClient::onDataAvailable(SocketClient* client)
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

UnixSocketClient::UnixSocketClientListener::UnixSocketClientListener(UnixSocketClient* mServer, int socketFd)
    : SocketListener(socketFd, false)
    , mServer(mServer)
{

}

UnixSocketClient::UnixSocketClientListener::~UnixSocketClientListener()
{

}

bool UnixSocketClient::UnixSocketClientListener::onDataAvailable(SocketClient* client)
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
