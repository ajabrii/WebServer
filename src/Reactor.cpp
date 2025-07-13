/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Reactor.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 17:35:45 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/13 17:31:16 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Reactor.hpp"
 /*
 === registerServer & add connection do the same thing one for server the other for the client ===
  ?this function register and add the servers fd to pollfd
  ?so we can start the listening on the expected events
 */
void Reactor::registerServer(HttpServer& server)
{
    const std::vector<int>& fds = server.getFds();

    for (size_t i = 0; i < fds.size(); ++i)
    {
        pollfd pfd;
        pfd.fd = fds[i];
        pfd.events = POLLIN;
        pfd.revents = 0;
        pollFDs.push_back(pfd);
        serverMap[fds[i]] = &server;
    }
}

void Reactor::addConnection(Connection* conn, HttpServer* server)
{
    pollfd pfd;
    pfd.fd = conn->getFd();
    pfd.events = POLLIN;
    pfd.revents = 0;
    pollFDs.push_back(pfd);
    connectionMap[conn->getFd()] = conn;
    clientToServerMap[conn->getFd()] = server;
}

void Reactor::removeConnection(int fd)
{
    for (std::vector<pollfd>::iterator it = pollFDs.begin(); it != pollFDs.end(); ++it)
    {
        if (it->fd == fd)
        {
            pollFDs.erase(it);
            break;
        }
    }
    std::map<int, Connection*>::iterator connIt = connectionMap.find(fd);
    if (connIt != connectionMap.end())
    {
        delete connIt->second;
        connectionMap.erase(connIt);
    }
    clientToServerMap.erase(fd);
}

/*
=== this function is where the multiplexing magic happens ===

*(x) poll() : lets the kernel watch multiple fds at once (monitoring for read/write events).
@ It blocks until at least one fd is ready (here we block forever with -1).
@ The kernel sets pfd.revents flags to show which events happened (e.g., POLLIN, POLLOUT).

*(x) We loop over pollFDs:
? If pfd.revents shows readable or writable, we create an Event struct to describe it.
@ This way, our reactor collects all ready events so we can handle them later.
*/
 //TODO: I should check if POLLERR or POLLHUB happend close the fd ... 
void Reactor::poll()
{
    readyEvents.clear();
    int ret = ::poll(pollFDs.data(), pollFDs.size(), -1);
    if (ret < 0)
        throw std::runtime_error("Error: poll failed");
    for (size_t i = 0; i < pollFDs.size(); ++i)
    {
        pollfd& pfd = pollFDs[i];
        if (pfd.revents & (POLLIN | POLLOUT))
        {
            Event evt;
            evt.fd = pfd.fd;
            evt.isReadable = (pfd.revents & POLLIN);
            evt.isWritable = (pfd.revents & POLLOUT);
            evt.isNewConnection = (serverMap.find(pfd.fd) != serverMap.end());
            readyEvents.push_back(evt);
        }
    }
}

//@ reactors Getters
std::vector<Event> Reactor::getReadyEvents() const
{
    return readyEvents;
}

Connection& Reactor::getConnection(int fd)
{
    std::map<int, Connection*>::iterator it = connectionMap.find(fd);
    if (it == connectionMap.end())
        throw std::runtime_error("Error: Connection not found for fd: " + std::to_string(fd));
    return *(it->second);
}

HttpServer* Reactor::getServerByListeningFd(int fd)
{
    std::map<int, HttpServer*>::iterator it = serverMap.find(fd);
    return (it != serverMap.end()) ? it->second : 0;
}

HttpServer* Reactor::getServerForClient(int clientFd)
{
    std::map<int, HttpServer*>::iterator it = clientToServerMap.find(clientFd);
    return (it != clientToServerMap.end()) ? it->second : 0;
}
