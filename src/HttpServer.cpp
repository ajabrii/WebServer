/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpServer.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 17:00:19 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/13 16:51:50 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "../includes/HttpServer.hpp"
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
=== this function is for server's network side setup ===

* open fd (socket) with socket():
? AF_INET : this flag says we are using IPv4.
? SOCK_STREAM : so we create a TCP socket (stream-oriented).
? 0 : tells the OS to choose the default protocol for this type of socket (for AF_INET + SOCK_STREAM, this means TCP).

* setsockopt() :
@ This function lets us configure socket options before binding.
? fd : the file descriptor returned earlier by socket().
? SOL_SOCKET : tells the kernel that the option applies to the socket layer itself.
? SO_REUSEADDR : the specific option we want to set.
@ This allows us to reuse the address (port) even if it's in TIME_WAIT after the server stops.
? Without this, after stopping the server, the port stays busy for a short time (TIME_WAIT state), so restarting immediately would fail with "address already in use".

* bind() :
@ This function assigns the socket to a specific local address and port, so the server knows on which IP and port to listen.
? sockaddr_in : is a struct that describes an IPv4 address.

struct sockaddr_in {
    sa_family_t    sin_family;     // must be AF_INET for IPv4
    in_port_t      sin_port;       // port number (needs to be in network byte order -> htons())
    struct in_addr sin_addr;       // IP address (binary form, network byte order)
    ...
};

* listen() :
@ This function marks the socket as passive, meaning it starts waiting for incoming connections.
? The second parameter (128) is the backlog: how many pending connections the kernel will queue up before it starts refusing new ones.
*/

void HttpServer::setup()
{
    for (size_t i = 0; i < config.port.size(); ++i)
    {
        int fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0)
            throw std::runtime_error("Failed to create socket");
        if (fcntl(fd, F_SETFL,O_NONBLOCK | FD_CLOEXEC) < 0) // set the client_fd to non_blocking too
            throw std::runtime_error("Error: fcntl failed");
        int opt = 1;
        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
            throw std::runtime_error("Error: setsockopt failed");
        sockaddr_in addr;
        std::memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(config.port[i]);
        addr.sin_addr.s_addr = inet_addr(config.host.c_str()); // this function is not int the in subject !!!

        if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
            throw std::runtime_error("Error: bind failed on port " + std::to_string(config.port[i]));

        if (listen(fd, 128) < 0)
            throw std::runtime_error("Error: listen failed");

        listen_fds.push_back(fd);

        std::cout << "\033[1;33m[*]\033[0m "
                  << "[HttpServer] Listening on [\033[1;36mhttp://"
                  << config.host << ":" << config.port[i] << "\033[0m]" << std::endl;
    }
}

// Return all listen fds so Reactor can register them
const std::vector<int>& HttpServer::getFds() const
{
    return listen_fds;
}

// Accept a connection from a specific listen_fd
Connection HttpServer::acceptConnection(int listen_fd) const
{
    sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_len);
    if (client_fd < 0)
        throw std::runtime_error("accept failed");
    return Connection(client_fd, client_addr);
}
//i think this function is not important, i'll remove it later on
HttpResponse HttpServer::handleRequest(const HttpRequest& request)
{
    (void)request;
    HttpResponse response;
    response.version = "HTTP/1.1";
    response.statusCode = 200;
    response.statusText = "OK";
    response.headers["Content-Type"] = "text/plain";
    response.body = "Hello from HttpServer!";
    return response;
}

const ServerConfig& HttpServer::getConfig() const
{
    return config;
}
