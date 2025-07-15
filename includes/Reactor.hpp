/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Reactor.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: baouragh <baouragh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 16:23:32 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/15 17:08:50 by baouragh         ###   ########.fr       */
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
    bool isNewConnection; // true if this fd is a listening fd
};

class Reactor {
private:
    std::vector<pollfd> pollFDs;
    std::map<int, HttpServer*> serverMap;         // listening_fd → HttpServer*
    std::map<int, Connection*> connectionMap;     // client_fd → Connection*
    std::map<int, HttpServer*> clientToServerMap; // client_fd → HttpServer*
    std::vector<Event> readyEvents;

public:
    void registerServer(HttpServer& server);           // register all fds of this server
    void addConnection(Connection* conn, HttpServer* server);
    void removeConnection(int fd);
    void poll();
    std::vector<Event> getReadyEvents() const;
    Connection& getConnection(int fd);
    HttpServer* getServerByListeningFd(int fd);
    HttpServer* getServerForClient(int clientFd);
    void watchCgi(Connection* conn);
    void unregisterFd(int fd);
};

#endif
