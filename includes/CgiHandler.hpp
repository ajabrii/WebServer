/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: baouragh <baouragh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/01 18:56:52 by baouragh          #+#    #+#             */
/*   Updated: 2025/08/05 16:23:17 by baouragh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

# include "HttpResponse.hpp"
# include "HttpRequest.hpp"
# include "RouteConfig.hpp"
# include "ServerConfig.hpp"
# include "HttpServer.hpp"
# include "CgiData.hpp"
# include "CgiState.hpp"

#include <sys/wait.h>
# include <iostream>
# include <vector>

# define GET "GET"
# define POST "POST"
# define DELETE "DELETE"

class CgiHandler
{
    private:
        ServerConfig _serverData;
        RouteConfig _route;
        HttpRequest _req;
        std::string _env_paths;
        CgiData _data;
        int _clientSocket;
        HttpResponse _errResponse;
    public:
        CgiHandler();
        CgiHandler(const HttpServer &server, const HttpRequest& req , const RouteConfig& route ,int clientSocket, std::string env_paths);
        CgiHandler(const CgiHandler& other);
        CgiHandler& operator=(const CgiHandler& other);
        ~CgiHandler();
        CgiState* execCgi(Connection &conn);
        CgiData check_cgi(void);
        char **set_env(Connection &conn);
        bool IsCgi();
};