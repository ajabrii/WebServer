#include "Server.hpp"

Server::Server(Socket &sock) : sock(sock) 
{
    serverAddress.sin_family = sock.getDomain();
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;
}
Server::~Server()
{

}

Socket& Server::getSocket() const
{
    return (sock);
}

// void Server::SetSocket(Socket sock)
// {
//     this->sock = sock;
// }
