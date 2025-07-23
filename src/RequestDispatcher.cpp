/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestDispatcher.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/30 09:14:07 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/23 15:13:23 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/RequestDispatcher.hpp"
#include "../includes/GetHandler.hpp"
#include "../includes/DeleteHandler.hpp"
#include "../includes/PostHandler.hpp"
#include "../includes/ResponseBuilder.hpp"
#include <iostream>
#include <fcntl.h>
#include <dirent.h>
#include <sstream>
#include <fstream>

bool RequestDispatcher::isHttpMethodAllowed(const HttpRequest &req, std::vector<std::string> Imethods)
{
    for (size_t i = 0; i < Imethods.size(); ++i)
    {
        if (req.method == Imethods[i])
        {
            return true;
        }
    }
    return false;
}

HttpResponse RequestDispatcher::dispatch(const HttpRequest &req, const RouteConfig &route, const ServerConfig &serverConfig) const
{
    if (!RequestDispatcher::isHttpMethodAllowed(req, route.allowedMethods))
    {
        return ResponseBuilder::createErrorResponse(405, "Method Not Allowed", serverConfig);
    }
    if (req.method == GET_M)
    {
        return GetHandler().handle(req, route, serverConfig);
    }
    else if (req.method == POST_M)
    {
        return PostHandler().handle(req, route, serverConfig);
    }
    else if (req.method == DELETE_M)
    {
        return DeleteHandler().handle(req, route, serverConfig);
    }
    else
    {
        return ResponseBuilder::createErrorResponse(501, "Not Implemented", serverConfig);
    }
}
