/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Reactor.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: baouragh <baouragh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 16:23:32 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/19 03:16:00 by baouragh         ###   ########.fr       */
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
    bool isPullHUP;
    bool isError;
    short errorType;
    Event();
};

class Reactor
{
    private:
        std::vector<pollfd> pollFDs;
        std::map<int, HttpServer*> serverMap;
        std::map<int, Connection*> connectionMap;
        std::map<int, HttpServer*> clientToServerMap;
        std::vector<Event> readyEvents;


    public:
        Reactor();
        ~Reactor();
        void registerServer(HttpServer& server);
        void addConnection(Connection* conn, HttpServer* server);
        void removeConnection(int fd);
        void poll();
        std::vector<Event> getReadyEvents() const;
        Connection& getConnection(int fd);
        HttpServer* getServerByListeningFd(int fd);
        HttpServer* getServerForClient(int clientFd);
        std::map<int, Connection*> getConnectionMap(void) const;
        void watchCgi(Connection* conn);
        void cgiRemover(Connection *conn);
        void cleanup();
        void cleanupTimedOutConnections(); // Cleanup connections that have timed out
};

#endif
