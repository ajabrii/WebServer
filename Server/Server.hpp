#ifndef SERVER_HPP
#define SERVER_HPP

#include "Socket.hpp"

class Server
{
    private:
        Socket sock;
        sockaddr_in serverAddress;
    public:
        Server(Socket sock);
        ~Server();
        Socket getSocket() const;

};


#endif