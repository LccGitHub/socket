/*
 */
#ifndef _SOCKETCLIENTCOMMAND_H
#define _SOCKETCLIENTCOMMAND_H


class SocketClient;
class SocketClientCommand 
{
public:
    virtual ~SocketClientCommand() { }
    virtual void runSocketCommand(SocketClient *client) = 0;
};

#endif
