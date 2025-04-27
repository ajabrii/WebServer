#include "Socket.hpp"


Socket::Socket(int domain, int type, int protocol)
{
    fd = socket(domain, type, protocol);
    if (fd == -1)
    {
        std::cerr << "Invalid File discreptor for socket!!" << std::endl;
        exit (errno);
    }
}

Socket::~Socket()
{
    close(fd);
}

int Socket::getFd()
{
    return (fd);
}