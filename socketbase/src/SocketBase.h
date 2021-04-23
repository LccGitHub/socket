/*
 */
#ifndef _SOCKETBASE_H
#define _SOCKETBASE_H
#include <string>
#include <unistd.h>


class SocketBase 
{
public:
    SocketBase(const std::string& ip, const uint16_t port){};
    virtual ~SocketBase() = default;
    virtual bool initSocket() = 0;
    virtual bool start() = 0;
    virtual bool stop() = 0;
    virtual bool send(const char* buf, const uint32_t length) = 0;
};

#endif /* _SOCKETBASE_H */
