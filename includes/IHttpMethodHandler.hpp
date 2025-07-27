/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IHttpMethodHandler.hpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: baouragh <baouragh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/01 18:11:07 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/27 11:18:01 by baouragh         ###   ########.fr       */
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
        virtual HttpResponse handle(const HttpRequest& req, const RouteConfig& route, const ServerConfig& serverConfig) const = 0;
};
