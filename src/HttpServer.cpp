/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpServer.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 17:00:19 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/07 12:25:26 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "../includes/HttpServer.hpp"
#include <stdexcept>

HttpServer::HttpServer(const ServerConfig& cfg) : config(cfg) {}

HttpServer::~HttpServer() {
    for (size_t i = 0; i < listen_fds.size(); ++i) {
        close(listen_fds[i]);
    }
}

void HttpServer::setup()
{
    for (size_t i = 0; i < config.port.size(); ++i) {
        int fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0)
            throw std::runtime_error("Failed to create socket");

        int opt = 1;
        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
            throw std::runtime_error("setsockopt failed");

        sockaddr_in addr;
        std::memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(config.port[i]);
        addr.sin_addr.s_addr = inet_addr(config.host.c_str()); // this function is not int the in subject !!!

        if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
            throw std::runtime_error("bind failed on port " + std::to_string(config.port[i]));

        if (listen(fd, 128) < 0)
            throw std::runtime_error("listen failed");

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
