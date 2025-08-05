/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: youness <youness@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/30 08:58:47 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/26 20:26:43 by youness          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "ServerConfig.hpp"
# include "RouteConfig.hpp"
# include "HttpServer.hpp"
#pragma once


class Router
{
    public:
         const RouteConfig* match(const HttpRequest& request, const ServerConfig& serverConfig) const;
};