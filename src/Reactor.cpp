/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Reactor.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 17:35:45 by ajabri            #+#    #+#             */
/*   Updated: 2025/06/30 15:46:52 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "../includes/Reactor.hpp"


void Reactor::registerServer(HttpServer& server) {
    pollfd pfd;
    pfd.fd = server.getFd();
    pfd.events = POLLIN;
    pfd.revents = 0;
    pollFDs.push_back(pfd);
    serverMap[server.getFd()] = &server;
}

void Reactor::addConnection(Connection* conn, HttpServer* server) {
    pollfd pfd;
    pfd.fd = conn->getFd();
    pfd.events = POLLIN; // only listen for read initially
    pfd.revents = 0;
    pollFDs.push_back(pfd);
    connectionMap[conn->getFd()] = conn;
    clientToServerMap[conn->getFd()] = server;
}

void Reactor::removeConnection(int fd) {
    for (std::vector<pollfd>::iterator it = pollFDs.begin(); it != pollFDs.end(); ++it) {
        if (it->fd == fd) {
            pollFDs.erase(it);
            break;
        }
    }
    std::map<int, Connection*>::iterator it = connectionMap.find(fd);
    if (it != connectionMap.end()) {
        delete it->second;  // free memory
        connectionMap.erase(it);
    }
    clientToServerMap.erase(fd);
}

void Reactor::poll() {
    readyEvents.clear();
    int ret = ::poll(pollFDs.data(), pollFDs.size(), -1);
    if (ret < 0)
        throw std::runtime_error("poll failed");

    for (size_t i = 0; i < pollFDs.size(); ++i) {
        pollfd& pfd = pollFDs[i];
        if (pfd.revents & (POLLIN | POLLOUT)) {
            Event evt;
            evt.fd = pfd.fd;
            evt.isReadable = (pfd.revents & POLLIN);
            evt.isWritable = (pfd.revents & POLLOUT);
            evt.isNewConnection = (serverMap.find(pfd.fd) != serverMap.end());
            readyEvents.push_back(evt);
        }
    }
}

std::vector<Event> Reactor::getReadyEvents() { return readyEvents; }

Connection& Reactor::getConnection(int fd) {
    std::map<int, Connection*>::iterator it = connectionMap.find(fd);
    if (it == connectionMap.end())
        throw std::runtime_error("Connection not found");
    return *(it->second);
}

HttpServer* Reactor::getServerByListeningFd(int fd) {
    std::map<int, HttpServer*>::iterator it = serverMap.find(fd);
    return it != serverMap.end() ? it->second : nullptr;
}

HttpServer* Reactor::getServerForClient(int clientFd) {
    std::map<int, HttpServer*>::iterator it = clientToServerMap.find(clientFd);
    return it != clientToServerMap.end() ? it->second : nullptr;
}


// // Implementation
// void Reactor::registerServer(HttpServer& server) {
//     pollfd pfd;
//     pfd.fd = server.getFd();
//     pfd.events = POLLIN;
//     pfd.revents = 0;
//     pollFDs.push_back(pfd);
//     serverFd = server.getFd(); // Store the server's file descriptor
//     serverMap[server.getFd()] = &server;
// }

// // void Reactor::addConnection(const Connection& conn) {
// //     pollfd pfd;
// //     pfd.fd = conn.getFd();
// //     pfd.events = POLLIN | POLLOUT;
// //     pfd.revents = 0;
// //     pollFDs.push_back(pfd);
// //     connectionMap[conn.getFd()] = conn;
// // }

// void Reactor::addConnection(Connection* conn, HttpServer* serv) {
//     pollfd pfd;
//     pfd.fd = conn->getFd();
//     pfd.events = POLLIN | POLLOUT;
//     pfd.revents = 0;
//     pollFDs.push_back(pfd);
//     connectionMap[conn->getFd()] = conn;
//     clientToservers[conn->getFd()] = &serv; // Map connection fd to server
// }


// void Reactor::removeConnection(int fd) {
//     for (std::vector<pollfd>::iterator it = pollFDs.begin(); it != pollFDs.end(); ++it) {
//         if (it->fd == fd) {
//             pollFDs.erase(it);
//             break;
//         }
//     }
//     std::map<int, Connection*>::iterator it = connectionMap.find(fd);
//     if (it != connectionMap.end()) {
//         delete it->second;  // free memory
//         connectionMap.erase(it);
//     }
// }


// void Reactor::poll() 
// {
//     readyEvents.clear(); //
//     int ret = ::poll(&pollFDs[0], pollFDs.size(), -1);
//     if (ret < 0) {
//         throw std::runtime_error("poll failed");
//     }
//     for (size_t i = 0; i < pollFDs.size(); ++i) {
//         pollfd& pfd = pollFDs[i];
//         if (pfd.revents & POLLIN || pfd.revents & POLLOUT) {
//             Event evt;
//             evt.fd = pfd.fd;
//             evt.isReadable = (pfd.revents & POLLIN) != 0;
//             evt.isWritable = (pfd.revents & POLLOUT) != 0;
//             evt.isNewConnection = (serverMap.find(pfd.fd) != serverMap.end());
//             readyEvents.push_back(evt);
//         }
//     }
// }

// std::vector<Event> Reactor::getReadyEvents() {
//     return readyEvents;
// }

// Connection& Reactor::getConnection(int fd) 
// {
//     std::map<int, Connection*>::iterator it = connectionMap.find(fd);
//     if (it == connectionMap.end())
//         throw std::runtime_error("Connection not found");
//     return (*it->second);
// }

// HttpServer* Reactor::getServer(int fd) {
//     std::map<int, HttpServer*>::iterator it = clientToservers.find(fd);
//     return it != serverMap.end() ? it->second : 0;
// }
