/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IHttpMethodHandler.hpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/01 18:11:07 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/12 10:19:26 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "HttpResponse.hpp"
#include "HttpRequest.hpp"
#include "RouteConfig.hpp"
#include <ctime>
#include <sstream>
#include <fstream>
#include "ServerConfig.hpp"


class IHttpMethodHandler
{
    
    public:
        virtual ~IHttpMethodHandler(){};
        // Legacy interface for backward compatibility
        // virtual HttpResponse handle(const HttpRequest& req, const RouteConfig& route) const = 0;
        // New interface with ServerConfig for error page support
        virtual HttpResponse handle(const HttpRequest& req, const RouteConfig& route, const ServerConfig& serverConfig) const = 0;
};
