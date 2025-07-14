/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpServer.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 17:00:19 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/14 16:38:12 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "../includes/HttpServer.hpp"
#include "../includes/Utils.hpp"
#include <stdexcept>

HttpServer::HttpServer(const ServerConfig& cfg) : config(cfg)
{
}

HttpServer::~HttpServer()
{
    for (size_t i = 0; i < listen_fds.size(); ++i)
    {
        close(listen_fds[i]);
    }
}

/*
=== this function is for server's network setup ===

* socket() : creates a new socket.
? AF_INET : IPv4
? SOCK_STREAM : TCP (connection-oriented)
? 0 : default protocol (TCP for AF_INET + SOCK_STREAM)

* setsockopt() : configures options before binding.
? SOL_SOCKET + SO_REUSEADDR : allow quick restart of server on same port, even if in TIME_WAIT.
? fd : socket file descriptor
? opt : usually set to 1 (enable)

* bind() : attach socket to local address+port.
? sockaddr_in : struct describing the IPv4 address
    - sin_family = AF_INET
    - sin_port = htons(port) // convert port to network byte order
    - sin_addr.s_addr = inet_addr(host) // convert host string to binary

* listen() : mark socket as passive to accept connections.
? 128 : backlog (max pending connections queue)

* fcntl() : set socket to non-blocking mode.
? F_GETFL : get current flags
? F_SETFL : set updated flags
? O_NONBLOCK : makes socket non-blocking so accept/read/write don’t block if no data.
*/

void HttpServer::setup()
{
    for (size_t i = 0; i < config.port.size(); ++i)
    {
        int fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0)
            throw std::runtime_error("Failed to create socket");
        if (fcntl(fd, F_SETFL,O_NONBLOCK | FD_CLOEXEC) < 0)
            throw std::runtime_error("Error: fcntl failed server_fd");
        int opt = 1;
        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
            throw std::runtime_error("Error: setsockopt failed");
        sockaddr_in addr;
        std::memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(config.port[i]);
        addr.sin_addr.s_addr = inet_addr(config.host.c_str()); // this function is not int the in subject !!!

        if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
            throw std::runtime_error("Error: bind failed on port " + Utils::toString(config.port[i]));

        if (listen(fd, 128) < 0)
            throw std::runtime_error("Error: listen failed");

        listen_fds.push_back(fd);

        std::cout << "\033[1;33m[*]\033[0m "
                  << "[HttpServer] Listening on [\033[1;36mhttp://"
                  << config.host << ":" << config.port[i] << "\033[0m]" << std::endl;
    }
}



/*
=== this function for accepting the client and return Connection Object ===
* accept function takes the socket that was listening for connection (server socket), takes sockaddr_in address (we don't need to fill the ip the port etc accept do that for us)
*take the the size of the struct
set the client_fd to non blocking
*/
Connection HttpServer::acceptConnection(int listen_fd) const
{
    sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_len);
    if (client_fd < 0)
        throw std::runtime_error("Error: accept failed");
    if (fcntl(client_fd, F_SETFL,O_NONBLOCK | FD_CLOEXEC) < 0)
    throw std::runtime_error("Error: fcntl failed client_fd");
    return Connection(client_fd, client_addr);
}
//@ Getters
const std::vector<int>& HttpServer::getFds() const
{
    return listen_fds;
}

const ServerConfig& HttpServer::getConfig() const
{
    return config;
}
