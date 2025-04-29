#include "Socket.hpp"


Socket::Socket(int domain, int type, int protocol)
{
    arry[DOMAIN] = domain;
    arry[TYPE] = type;
    arry[PROTOCOL] = protocol;
    arry[FD] = socket(domain, type, protocol);
    if (arry[FD] == -1)
    {
        std::cerr << "Invalid File discreptor for socket!!" << std::endl;
        exit (errno);
    }
}

int Socket::getDomain(void) const
{
    return (arry[DOMAIN]);
}

int Socket::getType(void) const
{
    return (arry[TYPE]);
}

int Socket::getProtocol(void) const
{
    return (arry[PROTOCOL]);
}

int Socket::getFd(void) const
{
    return (arry[FD]);
}

Socket::~Socket()
{
    close(arry[FD]);
}
