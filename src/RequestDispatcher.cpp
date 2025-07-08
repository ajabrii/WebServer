/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestDispatcher.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ytarhoua <ytarhoua@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/30 09:14:07 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/07 19:09:19 by ytarhoua         ###   ########.fr       */
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




HttpResponse RequestDispatcher::dispatch(const HttpRequest& req, const RouteConfig& route) const
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
        // Getha
        return GetHandler().handle(req, route);
    }
    else if (req.method == "POST")
    {
        // PostHandler postHandler;
        return PostHandler().handle(req, route);
    }
    else if (req.method == "DELETE")
    {
        // DeleteHandler deleteHandler;
        return DeleteHandler().handle(req, route);
    }
    else
    {
        HttpResponse res;
        res.statusCode = 501; // Not Implemented
        res.statusText = "Not Implemented";
        res.body = "HTTP method not implemented.";
        return res;
    }
}

