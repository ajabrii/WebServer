/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GetHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/01 18:20:34 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/01 18:44:37 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "../includes/GetHandler.hpp"
#include "../includes/RequestDispatcher.hpp"
// # include "../includes/HttpResponse.hpp"
# include <iostream>
# include <fcntl.h>
#include <dirent.h>
#include <sstream>
# include <fstream>

std::string clean_line(std::string line);
// HttpResponse RequestDispatcher::handleRedirect(const std::string &redirectUrl) const;

GetHandler::GetHandler(){ }

GetHandler::~GetHandler() {}

HttpResponse &GetHandler::handle(const HttpRequest &req, const RouteConfig& route) const
{
    // TODO
    //*-> GET method logic implimented here;
    // 1. Redirects have highest priority
    if (!route.redirect.empty()) {
        std::cout << "--------------------->[*] Redirecting to: " << route.redirect << std::endl;
        return handleRedirect(route.redirect);
    }

    // 2. Check if method is allowed
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


    // 3. Compose file path: route.root + request.uri
    // std::string filePath = route.path + req.uri;
    // std::cout << "file path   ::" << route.root << "-----" << req.uri << "\n";
    // 4. Directory listing if enabled
    std::string filePath = clean_line(route.root) + clean_line(req.uri);
    // std::cout << "request path: " << req.uri << std::endl;
    // std::cout << "match root: " << route.root << std::endl;
    // std::cout << "File path: " << filePath << std::endl;
    // std::cout << "match->directory_listing: " << match->directory_listing << std::endl
    if (route.autoindex) {
        // std::cout << "--2------------------>[*] Redirecting to: " << route.redirect << std::endl;
        return handleDirectoryListing(filePath, req.uri);
    }

    // 5. Default: try serving static file
    return serveStaticFile(filePath);
}