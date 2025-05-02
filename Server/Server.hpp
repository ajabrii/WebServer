#ifndef SERVER_HPP
#define SERVER_HPP

#include "../Socket/Socket.hpp"

class Server
{
    private:
        Socket &sock;
        sockaddr_in serverAddress;
    public:
        Server(Socket &sock);
        ~Server();
        Socket& getSocket() const;
        sockaddr_in getServerAddress() const
        {
            return serverAddress;
        }
        void setServerAddress(sockaddr_in addr)
        {
            serverAddress = addr;
        }
        // void SetSocket(Socket sock);

};


#endif