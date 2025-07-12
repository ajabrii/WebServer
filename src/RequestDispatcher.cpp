/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestDispatcher.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/30 09:14:07 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/12 10:17:19 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "../includes/RequestDispatcher.hpp"
#include "../includes/GetHandler.hpp"
#include "../includes/DeleteHandler.hpp"
#include "../includes/PostHandler.hpp"
# include <iostream>
# include <fcntl.h>
#include <dirent.h>
#include <sstream>
# include <fstream>
std::string clean_line(std::string line);




HttpResponse RequestDispatcher::dispatch(const HttpRequest& req, const RouteConfig& route, const ServerConfig& serverConfig) const
{
    
    bool allowed = false;
    for (size_t i = 0; i < route.allowedMethods.size(); ++i) {
        if (req.method == route.allowedMethods[i]) {
            allowed = true;
            break;
        }
    }
    if (!allowed) {
        
        HttpResponse res;
        res.statusCode = 405;
        res.statusText = "Method Not Allowed";
        res.body = "HTTP method not allowed for this route.";
        std::cout << "--2------------------>[*] Redirecting to: " << route.redirect << std::endl;

        return res;
    }
    if (req.method == "GET")
    {
        return GetHandler().handle(req, route, serverConfig);
    }
    else if (req.method == "POST")
    {
        return PostHandler().handle(req, route);
    }
    else if (req.method == "DELETE")
    {
        return DeleteHandler().handle(req, route, serverConfig);
    }
    else
    {
        HttpResponse res;
        res.statusCode = 501; // Not Implemented
        res.headers["Content-Type"] = "text/html; charset=UTF-8";
        res.statusText = "Not Implemented";
        res.body = "HTTP method not implemented.";
        return res;
    }
}

