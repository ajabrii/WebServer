/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Reactor.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 16:23:32 by ajabri            #+#    #+#             */
/*   Updated: 2025/06/30 15:46:23 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// Reactor.hpp
#ifndef REACTOR_HPP
#define REACTOR_HPP

#include "HttpServer.hpp"
#include "Connection.hpp"
#include <vector>
#include <map>
#include <poll.h>

struct Event {
    int fd;
    bool isReadable;
    bool isWritable;
    bool isNewConnection;
};

class Reactor
{
private:
    std::vector<pollfd> pollFDs;
    std::map<int, HttpServer*> serverMap;         // listening_fd → server
    std::map<int, Connection*> connectionMap;     // client_fd → Connection*
    std::map<int, HttpServer*> clientToServerMap; // client_fd → server
    std::vector<Event> readyEvents;

public:
    void registerServer(HttpServer& server);
    void addConnection(Connection* conn, HttpServer* server);
    void removeConnection(int fd);
    void poll();
    std::vector<Event> getReadyEvents();
    Connection& getConnection(int fd);
    HttpServer* getServerByListeningFd(int fd);       // for new connections
    HttpServer* getServerForClient(int clientFd);     // for existing client fd
};


#endif
