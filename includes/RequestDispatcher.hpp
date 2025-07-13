/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestDispatcher.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/30 09:13:08 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/13 19:01:05 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_DISPATCHER_HPP
#define REQUEST_DISPATCHER_HPP

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "RouteConfig.hpp"
#include "ServerConfig.hpp"
# define GET_M "GET"
#define POST_M "POST"
# define DELETE_M "DELETE"

class RequestDispatcher
{
    public:
        static bool isHttpMethodAllowed(const HttpRequest& req,std::vector<std::string> Imethods);
        HttpResponse dispatch(const HttpRequest &request, const RouteConfig &route, const ServerConfig &serverConfig) const;
};

#endif
