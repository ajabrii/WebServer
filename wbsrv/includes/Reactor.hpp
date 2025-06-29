/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Reactor.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 16:23:32 by ajabri            #+#    #+#             */
/*   Updated: 2025/06/26 17:40:15 by ajabri           ###   ########.fr       */
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
        std::map<int, HttpServer*> serverMap;
        std::map<int, Connection> connectionMap;
        std::vector<Event> readyEvents;

    public:
        void registerServer(HttpServer& server);
        void addConnection(const Connection& conn);
        void removeConnection(int fd);
        void poll();
        std::vector<Event> getReadyEvents();
        Connection& getConnection(int fd);
        HttpServer* getServer(int fd);
};

#endif
