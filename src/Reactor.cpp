/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Reactor.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 17:35:45 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/07 10:01:17 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Reactor.hpp"

void Reactor::registerServer(HttpServer& server) {
    const std::vector<int>& fds = server.getFds();
    for (size_t i = 0; i < fds.size(); ++i) {
        pollfd pfd;
        pfd.fd = fds[i];
        pfd.events = POLLIN;
        pfd.revents = 0;
        pollFDs.push_back(pfd);
        serverMap[fds[i]] = &server;  // each listen_fd → server
    }
}

void Reactor::addConnection(Connection* conn, HttpServer* server) {
    pollfd pfd;
    pfd.fd = conn->getFd();
    pfd.events = POLLIN; // listen for read events
    pfd.revents = 0;
    pollFDs.push_back(pfd);
    connectionMap[conn->getFd()] = conn;
    clientToServerMap[conn->getFd()] = server;
}

void Reactor::removeConnection(int fd) {
    // Remove fd from pollFDs
    for (std::vector<pollfd>::iterator it = pollFDs.begin(); it != pollFDs.end(); ++it) {
        if (it->fd == fd) {
            pollFDs.erase(it);
            break;
        }
    }

    // Free and remove Connection*
    std::map<int, Connection*>::iterator connIt = connectionMap.find(fd);
    if (connIt != connectionMap.end()) {
        delete connIt->second;
        connectionMap.erase(connIt);
    }

    // Remove client-to-server mapping
    clientToServerMap.erase(fd);
}

void Reactor::poll() {
    readyEvents.clear();
    int ret = ::poll(pollFDs.data(), pollFDs.size(), -1);
    if (ret < 0) {
        throw std::runtime_error("poll failed");
    }

    for (size_t i = 0; i < pollFDs.size(); ++i) {
        pollfd& pfd = pollFDs[i];
        if (pfd.revents & (POLLIN | POLLOUT)) {
            Event evt;
            evt.fd = pfd.fd;
            evt.isReadable = (pfd.revents & POLLIN);
            evt.isWritable = (pfd.revents & POLLOUT);
            evt.isNewConnection = (serverMap.find(pfd.fd) != serverMap.end()); // if it's listening fd
            readyEvents.push_back(evt);
        }
    }
}

std::vector<Event> Reactor::getReadyEvents() const {
    return readyEvents;
}

Connection& Reactor::getConnection(int fd) {
    std::map<int, Connection*>::iterator it = connectionMap.find(fd);
    if (it == connectionMap.end())
        throw std::runtime_error("Connection not found for fd: " + std::to_string(fd));
    return *(it->second);
}

HttpServer* Reactor::getServerByListeningFd(int fd) {
    std::map<int, HttpServer*>::iterator it = serverMap.find(fd);
    return (it != serverMap.end()) ? it->second : nullptr;
}

HttpServer* Reactor::getServerForClient(int clientFd) {
    std::map<int, HttpServer*>::iterator it = clientToServerMap.find(clientFd);
    return (it != clientToServerMap.end()) ? it->second : nullptr;
}
