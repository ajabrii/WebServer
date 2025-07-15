/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestDispatcher.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/30 09:14:07 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/14 14:29:33 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "../includes/RequestDispatcher.hpp"
#include "../includes/GetHandler.hpp"
#include "../includes/DeleteHandler.hpp"
#include "../includes/PostHandler.hpp"
# include "../includes/Errors.hpp"
# include "../includes/Utils.hpp"

# include <iostream>
# include <fcntl.h>
#include <dirent.h>
#include <sstream>
# include <fstream>


bool RequestDispatcher::isHttpMethodAllowed(const HttpRequest& req, std::vector<std::string> Imethods)
{
    for (size_t i = 0; i < Imethods.size(); ++i)
    {
        if (req.method == Imethods[i]) {
            return true;
        }
    }
    return false;
}

HttpResponse RequestDispatcher::dispatch(const HttpRequest& req, const RouteConfig& route, const ServerConfig& serverConfig) const
{
    HttpResponse res;
    if (!RequestDispatcher::isHttpMethodAllowed(req, route.allowedMethods))
    {
        res.statusCode = 405;
        res.headers["content-type"] = "text/html; charset=UTF-8";
        res.statusText = "Method Not Allowed";
        res.body = Error::loadErrorPage(res.statusCode, serverConfig);

        return res;
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
        res.statusCode = 501;
        res.headers["content-type"] = "text/html; charset=UTF-8";
        res.statusText = "Not Implemented";
        res.body = Error::loadErrorPage(res.statusCode, serverConfig);
        return res;
    }
}

