/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Reactor.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 17:35:45 by ajabri            #+#    #+#             */
/*   Updated: 2025/08/06 12:14:35 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Reactor.hpp"
#include "../includes/Utils.hpp"

Reactor::Reactor() {}

Reactor::~Reactor() {
    cleanup();
}

Event::Event() : fd(-1), isReadable(false), isWritable(false),
              isNewConnection(false), isError(false), errorType(0) {}

void Reactor::cleanup()
{
    for (std::map<int, Connection*>::iterator it = connectionMap.begin(); it != connectionMap.end(); ++it)
    {
        Connection* conn = it->second;

        if (conn->getCgiState())
        {
            delete conn->getCgiState();
        }
        delete conn;
    }
    connectionMap.clear();
    clientToServerMap.clear();
    pollFDs.clear();
    serverMap.clear();
}

void Reactor::cleanupTimedOutConnections()
{
    std::vector<int> timedOutFds;

    for (std::map<int, Connection*>::iterator it = connectionMap.begin(); it != connectionMap.end(); ++it) {
        if (it->second->isKeepAlive() && it->second->isTimedOut())
            timedOutFds.push_back(it->first);
    }

    for (size_t i = 0; i < timedOutFds.size(); ++i) {
        std::cout << "\033[1;33m[TIMEOUT]\033[0m Connection " << timedOutFds[i] << " timed out" << std::endl;
        removeConnection(timedOutFds[i]);
    }
}

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
void Reactor::cgiRemover(Connection *conn)
{
    int fds[2];
    CgiState *cgi = conn->getCgiState();
    if (cgi == NULL)
        return;
    fds[0] = cgi->output_fd;
    fds[1] = conn->getFd();

    for (std::vector<pollfd>::iterator it = pollFDs.begin(); it != pollFDs.end();)
    {
        if (it->fd == fds[0] || (it->fd == fds[1] && !conn->isKeepAlive()))
        {
            it = pollFDs.erase(it);
        }
        else
        {
            ++it;
        }
    }
    for (int i = 0; i < 2; ++i)
    {
        if (i == 0 || (i == 1 && !conn->isKeepAlive()))
        {
            std::map<int, Connection*>::iterator connIt = connectionMap.find(fds[i]);
            if (connIt != connectionMap.end())
            {
                connectionMap.erase(connIt);
            }
        }
    }
    clientToServerMap.erase(fds[0]);
    if (!conn->isKeepAlive())
        clientToServerMap.erase(fds[1]);
    if (cgi->output_fd != -1)
    {
        close(cgi->output_fd);
        cgi->output_fd = -1;
    }
    if (cgi->input_fd != -1)
    {
        close(cgi->input_fd);
        cgi->input_fd = -1;
    }
    if (conn->isKeepAlive())
    {
        conn->setKeepAlive(true);
        conn->incrementRequestCount();
        conn->resetForNextRequest();
        conn->updateLastActivity();
        std::cout << "\033[1;32m[+]\033[0m Connection kept alive (request #" << conn->getRequestCount() << ")" << std::endl;
    }
    else
        conn->closeConnection();
    std::cerr << "\033[1;34m[CGI]\033[0m CGI state cleaned up for fd: " << conn->getFd() << std::endl;
}


void Reactor::removeConnection(int fd)
{
    if (fd == -1)
        return;
    std::map<int, Connection*>::iterator connIt = connectionMap.find(fd);
    if (connIt != connectionMap.end())
    {
        if (connIt->second->getCgiState())
        {
            cgiRemover(connIt->second);
            return;
        }
        delete connIt->second;
        connectionMap.erase(connIt);
    }
    for (std::vector<pollfd>::iterator it = pollFDs.begin(); it != pollFDs.end(); ++it)
    {
        if (it->fd == fd)
        {
            pollFDs.erase(it);
            break;
        }
    }
    clientToServerMap.erase(fd);
    std::cout << "\033[1;31m[-]\033[0m " << "TCP connection closed" << std::endl;
}

void Reactor::poll()
{
    readyEvents.clear();
    int ret = ::poll(pollFDs.data(), pollFDs.size(), 1000);
    if (ret < 0)
    {
        if ( errno == EINTR )
            return;
        throw std::runtime_error("Error: poll failed");
    }
    for (size_t i = 0; i < pollFDs.size(); ++i)
    {
        pollfd& pfd = pollFDs[i];
        if (pfd.revents == 0)
            continue;
        Event evt;
        evt.fd = pfd.fd;

        if (pfd.revents & (POLLERR | POLLNVAL))
        {
            evt.isError = true;
            evt.errorType = pfd.revents;
            readyEvents.push_back(evt);
            continue;
        }
        if (pfd.revents & (POLLIN | POLLOUT))
        {
            evt.isReadable = (pfd.revents & POLLIN);
            evt.isWritable = (pfd.revents & POLLOUT);
            evt.isNewConnection = (serverMap.find(pfd.fd) != serverMap.end());
            readyEvents.push_back(evt);
        }
        if (pfd.revents & POLLHUP)
        {
            evt.isPullHUP = true;
            readyEvents.push_back(evt);
        }
    }
}

std::vector<Event> Reactor::getReadyEvents() const
{
    return readyEvents;
}

std::vector<pollfd> Reactor::getPollFds() const
{
    return pollFDs;
}


Connection& Reactor::getConnection(int fd)
{
    std::map<int, Connection*>::iterator it = connectionMap.find(fd);
    if (it == connectionMap.end())
        throw std::runtime_error("Error: Connection not found for fd: " + Utils::toString(fd));
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

void Reactor::watchCgi(Connection* conn)
{
    if (!conn)
        return;
    CgiState *cgiState = conn->getCgiState();
    if (!cgiState)
        return;

    pollfd pfdOut;
    pfdOut.fd = cgiState->output_fd;
    pfdOut.events = POLLIN;
    pollFDs.push_back(pfdOut);
    connectionMap[cgiState->output_fd] = conn;
}

std::map<int, Connection*> Reactor::getConnectionMap(void) const
{
    return connectionMap;
}