#include "Socket.hpp"


Socket::Socket()
{
    // std::cout << "Socket constructor called" << std::endl;
    arry[DOMAIN] = AF_INET;
    arry[TYPE] = SOCK_STREAM;
    arry[PROTOCOL] = 0;
}

Socket::Socket(int domain, int type, int protocol) 
{
    std::cout << "Socket constructor called" << std::endl;
    arry[DOMAIN] = domain;
    arry[TYPE] = type;
    arry[PROTOCOL] = protocol;
    // fd = socket(domain, type, protocol);
    // if (fd == -1)
    // {
    //     std::cerr << "Invalid File discreptor for socket!!" << std::endl;
    //     exit (errno);
    // }
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
    return (fd);
}

Socket::~Socket()
{
    // std::cout << "Socket destructor called" << std::endl;
    // close(fd);
}
