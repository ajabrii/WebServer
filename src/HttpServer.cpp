/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpServer.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 17:00:19 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/30 11:00:09 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/HttpServer.hpp"
#include "../includes/Utils.hpp"
#include <stdexcept>
#include <netdb.h>

HttpServer::HttpServer(const ServerConfig &cfg) : config(cfg)   {}

HttpServer::~HttpServer()
{
    for (size_t i = 0; i < listen_fds.size(); ++i)
    {
        close(listen_fds[i]);
    }
}

void HttpServer::setup()
{
    for (size_t i = 0; i < config.port.size(); ++i)
    {
        int fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0)
        {
            for (size_t j = 0; j < listen_fds.size(); ++j)
                close(listen_fds[j]);
            listen_fds.clear();
            throw std::runtime_error("Failed to create socket");
        }
        if (fcntl(fd, F_SETFL, O_NONBLOCK | FD_CLOEXEC) < 0)
        {
            close(fd);
            for (size_t j = 0; j < listen_fds.size(); ++j)
                close(listen_fds[j]);
            listen_fds.clear();
            throw std::runtime_error("Error: fcntl failed server_fd");
        }
        int opt = 1;
        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        {
            close(fd);
            for (size_t j = 0; j < listen_fds.size(); ++j)
                close(listen_fds[j]);
            listen_fds.clear();
            throw std::runtime_error("Error: setsockopt failed");
        }
        struct addrinfo hints, *result;
        std::memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;
        std::string port_str = Utils::toString(config.port[i]);
        int status = getaddrinfo(config.host.c_str(), port_str.c_str(), &hints, &result);
        if (status != 0)
        {
            close(fd);
            for (size_t j = 0; j < listen_fds.size(); ++j)
                close(listen_fds[j]);
            listen_fds.clear();
            throw std::runtime_error("Error: getaddrinfo failed: " + std::string(gai_strerror(status)));
        }
        if (bind(fd, result->ai_addr, result->ai_addrlen) < 0)
        {
            close(fd);
            freeaddrinfo(result);
            for (size_t j = 0; j < listen_fds.size(); ++j)
                close(listen_fds[j]);
            listen_fds.clear();
            throw std::runtime_error("Error: bind failed on port " + Utils::toString(config.port[i]));
        }
        freeaddrinfo(result);
        if (listen(fd, CLIENT_QUEUE) < 0)
        {
            close(fd);
            for (size_t j = 0; j < listen_fds.size(); ++j)
                close(listen_fds[j]);
            listen_fds.clear();
            throw std::runtime_error("Error: listen failed on port " + Utils::toString(config.port[i]));
        }

        listen_fds.push_back(fd);
        ServerLog(i);
    }
}

Connection HttpServer::acceptConnection(int listen_fd) const
{
    sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &client_len);
    if (client_fd < 0)
        throw std::runtime_error("Error: accept failed");
    if (fcntl(client_fd, F_SETFL, O_NONBLOCK | FD_CLOEXEC) < 0)
        throw std::runtime_error("Error: fcntl failed client_fd");
    return Connection(client_fd, client_addr);
}
//@ Getters
const std::vector<int> &HttpServer::getFds() const
{
    return listen_fds;
}

const ServerConfig &HttpServer::getConfig() const
{
    return config;
}

void HttpServer::ServerLog(size_t i) const
{
    std::cout << "\033[1;33m[*]\033[0m "
              << "[HttpServer] Listening on [\033[1;36mhttp://"
              << config.host << ":" << config.port[i] << "\033[0m]" << std::endl;
}
