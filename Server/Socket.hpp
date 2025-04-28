#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <sys/types.h>
#include <sys/socket.h>
# include <iostream>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>

#define DOMAIN

class Socket
{
    private:
        int arry[4]
        // int domain;
        // int type;
        // int protocol;
        // int fd;
    public:
        Socket(int domain, int type, int protocol);
        ~Socket();
        int getFd();
};

#endif