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
#define FD 4 //! remove this later on

class Socket
{
    private:
        int arry[5];
        int fd;
        public:
        Socket();
        Socket(int domain, int type, int protocol);
        ~Socket();
        int getDomain(void) const;
        int getType(void) const;
        int getProtocol(void) const;
        int getFd(void) const;
        void setSocket(int domain, int type, int protocol)
        {
            fd = socket(domain, type, protocol);
            if (fd == -1)
            {
                std::cerr << "Invalid File discreptor for socket!!" << std::endl;
                exit (errno); //clean and exit
            }
        }
        
        Socket& operator=(const Socket& other)
        {
            if (this != &other)
            {
                this->fd = other.fd;
                for (int i = 0; i < 4; ++i)
                    this->arry[i] = other.arry[i];
            }
            return *this;
        }

        
};

#endif