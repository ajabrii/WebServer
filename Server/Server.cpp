#include "Server.hpp"

Server::Server(Socket sock) : sock(sock) {}
Server::~Server()
{

}

Socket Server::getSocket() const
{
    return (sock);
}