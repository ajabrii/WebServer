/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpServer.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 16:14:39 by ajabri            #+#    #+#             */
/*   Updated: 2025/06/26 16:19:31 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


// HttpServer.hpp
#ifndef HTTP_SERVER_HPP
#define HTTP_SERVER_HPP

#include "ServerConfig.hpp"
#include "Connection.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include <vector>
#include <string>
#include <map>
#include <netinet/in.h>
#include <unistd.h>

class HttpServer {
private:
    ServerConfig config;
    int listen_fd;
    sockaddr_in server_addr;

public:
    HttpServer(const ServerConfig& cfg);
    ~HttpServer();

    void setup();
    int getFd() const;
    Connection acceptConnection() const;
    HttpResponse handleRequest(const HttpRequest& request);
    const ServerConfig& getConfig() const;
};

#endif
