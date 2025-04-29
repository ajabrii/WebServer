#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <sys/types.h>
#include <sys/socket.h>
# include <iostream>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>

#define DOMAIN 0
#define TYPE 1
#define PROTOCOL 2
#define FD 3

class Socket
{
    private:
        int arry[4];
    public:
        Socket(int domain, int type, int protocol);
        ~Socket();
        int getDomain(void) const;
        int getType(void) const;
        int getProtocol(void) const;
        int getFd(void) const;

        
};

#endif