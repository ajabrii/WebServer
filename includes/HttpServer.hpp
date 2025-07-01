/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpServer.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 16:14:39 by ajabri            #+#    #+#             */
/*   Updated: 2025/06/30 10:50:49 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


// HttpServer.hpp
#ifndef HTTP_SERVER_HPP
#define HTTP_SERVER_HPP

#include "ServerConfig.hpp"
#include "Connection.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include <netinet/in.h>
#include <stdexcept> 
# include <iostream>
# include <fstream>
# include <string>
#include <cstdlib>
# include <vector>
# include <cctype>
# include <ctime>
# include <map>
#include <cstring>      // For memset
#include <unistd.h>     // For close()
#include <sys/socket.h> // For socket functions // For sockaddr_in
#include <arpa/inet.h>  // For inet_ntoa()
#include <sstream> // for stringstream() to spil string
# include <poll.h>
# include <fcntl.h>


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
