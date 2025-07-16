/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Reactor.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: baouragh <baouragh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 17:35:45 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/16 10:09:44 by baouragh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Reactor.hpp"
#include "../includes/Utils.hpp"

Reactor::Reactor() {
    // Constructor
}

Reactor::~Reactor() {
    cleanup();
}

Event::Event() : fd(-1), isReadable(false), isWritable(false), 
              isNewConnection(false), isError(false), errorType(0) {}

void Reactor::cleanup() {
    // Clean up all connections
    for (std::map<int, Connection*>::iterator it = connectionMap.begin(); it != connectionMap.end(); ++it) {
        delete it->second;
    }
    connectionMap.clear();
    clientToServerMap.clear();
    pollFDs.clear();
    serverMap.clear();
}

void Reactor::cleanupTimedOutConnections()
{
    std::vector<int> timedOutFds;
    
    // Find timed out connections
    for (std::map<int, Connection*>::iterator it = connectionMap.begin(); it != connectionMap.end(); ++it) {
        if (it->second->isKeepAlive() && it->second->isTimedOut()) {
            timedOutFds.push_back(it->first);
        }
    }
    
    // Remove timed out connections
    for (size_t i = 0; i < timedOutFds.size(); ++i) {
        std::cout << "\033[1;33m[TIMEOUT]\033[0m Connection " << timedOutFds[i] << " timed out" << std::endl;
        removeConnection(timedOutFds[i]);
    }
}
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
    int ret = ::poll(pollFDs.data(), pollFDs.size(), 1000); // 1 second timeout for keep-alive cleanup
    if (ret < 0)
        throw std::runtime_error("Error: poll failed");
    for (size_t i = 0; i < pollFDs.size(); ++i)
    {
        pollfd& pfd = pollFDs[i];
        if (pfd.revents == 0) continue; // No events on this fd
        
        Event evt;
        evt.fd = pfd.fd;
        
        // Check for error conditions FIRST (highest priority)
        if (pfd.revents & (POLLERR | POLLHUP | POLLNVAL))
        {
            evt.isError = true;
            evt.errorType = pfd.revents;
            readyEvents.push_back(evt);
            continue; // Don't process other events for this fd
        }
        
        // Check for normal events
        if (pfd.revents & (POLLIN | POLLOUT))
        {
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

    // Watch CGI output (stdout)
    pollfd pfdOut;
    pfdOut.fd = cgiState->output_fd;
    pfdOut.events = POLLIN;
    pollFDs.push_back(pfdOut);

    // Watch CGI input (stdin) only for POST
    if (cgiState->input_fd != -1) 
    {
        pollfd pfdIn;
        pfdIn.fd = cgiState->input_fd;
        pfdIn.events = POLLOUT;
        pollFDs.push_back(pfdIn);
    }

    // Optional: associate CGI fds with connection
    connectionMap[cgiState->output_fd] = conn;
    if (cgiState->input_fd != -1)
        connectionMap[cgiState->input_fd] = conn;
}
