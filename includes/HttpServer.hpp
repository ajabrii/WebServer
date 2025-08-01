/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpServer.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 16:14:39 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/28 13:25:33 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */



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
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sstream>
# include <poll.h>
# include <fcntl.h>

# define CLIENT_QUEUE 128
# define NEW_CLIENT_CON "\033[1;32m[+]\033[0m New client connected\n"

class HttpServer
{
    private:
        ServerConfig config;
        std::vector<int> listen_fds;
    public:
        HttpServer(const ServerConfig& cfg);
        ~HttpServer();

        void setup();
        const std::vector<int>& getFds() const;
        Connection acceptConnection(int listen_fd) const;
        const ServerConfig& getConfig() const;
        void ServerLog(size_t i ) const;
};


#endif
