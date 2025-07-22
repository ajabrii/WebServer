/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IHttpMethodHandler.hpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/01 18:11:07 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/22 12:48:13 by ajabri           ###   ########.fr       */
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
