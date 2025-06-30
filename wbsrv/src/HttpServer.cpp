/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpServer.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 17:00:19 by ajabri            #+#    #+#             */
/*   Updated: 2025/06/30 06:42:14 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "../includes/HttpServer.hpp"
#include <stdexcept>


HttpServer::HttpServer(const ServerConfig& cfg) : config(cfg)
{
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0)
        throw std::runtime_error("Failed to create socket");

    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(cfg.port);
    server_addr.sin_addr.s_addr = inet_addr(cfg.host.c_str());
}

HttpServer::~HttpServer() {
    close(listen_fd);
}

void HttpServer::setup() {
    int opt = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        throw std::runtime_error("setsockopt failed");

    if (bind(listen_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
        throw std::runtime_error("bind failed");

    if (listen(listen_fd, 128) < 0)
        throw std::runtime_error("listen failed");

    std::cout << "[+][HttpServer] Listening on http://" << config.host << ":" << config.port << std::endl;
}

int HttpServer::getFd() const {
    return listen_fd;
}

Connection HttpServer::acceptConnection() const {
    sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_len);
    if (client_fd < 0)
        throw std::runtime_error("accept failed");

    return Connection(client_fd, client_addr);
}

HttpResponse HttpServer::handleRequest(const HttpRequest& request) {
    (void)request;
    HttpResponse response;
    response.version = "HTTP/1.1";
    response.statusCode = 200;
    response.statusText = "OK";
    response.headers["Content-Type"] = "text/plain";
    response.body = "Hello from HttpServer!";
    // response.headers["Content-Length"] = std::to_string(response.body.size());
    return response;
}

const ServerConfig& HttpServer::getConfig() const {
    return config;
}

