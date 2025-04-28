#include "Socket.hpp"


Socket::Socket(int domain, int type, int protocol) : domain(domain) , type(type), protocol(protocol)
{
    fd = socket(domain, type, protocol);
    if (fd == -1)
    {
        std::cerr << "Invalid File discreptor for socket!!" << std::endl;
        exit (errno);
    }
}

int Socket::getDomain(void) const
{

}

int Socket::getDomain(void) const
{
    // SOCK_NONBLOCK
}

int Socket::getDomain(void) const
{

}

Socket::~Socket()
{
    close(fd);
}

int Socket::getFd()
{
    return (fd);
}