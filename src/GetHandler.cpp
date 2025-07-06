/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GetHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/01 18:20:34 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/04 16:10:37 by ajabri           ###   ########.fr       */
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

HttpResponse GetHandler::handle(const HttpRequest &req, const RouteConfig& route) const
{
    // TODO
    //*-> GET method logic implimented here;
    // 1. Redirects have highest priority
    if (!route.redirect.empty()) {
        std::cout << "--------------------->[*] Redirecting to: " << route.redirect << std::endl;
        return handleRedirect(route.redirect);
    }

    // 3. Compose file path: route.root + request.uri
    // std::string filePath = route.path + req.uri;
    // std::cout << "========================================================================uri" << req.uri << "\n";
    // 4. Directory listing if enabled
    // std::string filePath = clean_line(route.root) + clean_line(req.uri);
    std::string filePath = clean_line(route.root);

    // std::cout << "========================================================================request path: " << route.path << std::endl;
    // std::cout << "========================================================================match root: " << route.root << std::endl;
    // std::cout << "==============================================>File path: " << filePath << std::endl;
    // std::cout << "match->directory_listing: " << match->directory_listing << std::endl
    if (route.directory_listing) {
        // std::cout << "--2------------------>[*] Redirecting to: " << route.redirect << std::endl;
        return handleDirectoryListing(filePath, req.uri);
    }

    // 5. Default: try serving static file
    return serveStaticFile(filePath);
}



HttpResponse GetHandler::handleRedirect(const std::string& redirectUrl) const {
    HttpResponse res;
    res.statusCode = 301;
    res.statusText = "Moved Permanently";
    res.headers["Location"] = redirectUrl;
    res.body = "<html><body><h1>301 Moved Permanently</h1></body></html>";
    return res;
}

HttpResponse GetHandler::serveStaticFile(std::string& filePath) const 
{
    // filePath = filePath.substr(0, filePath.length() - 1); // Remove trailing slash if exists
    // filePath = "./index.html"; // Remove trailing slash if exists

    std::cout << "---3----------------->[*] Serving static file: `" << filePath << "'"<<std::endl;
    // filePath = "index.html";
    std::ifstream file(filePath.c_str());
    HttpResponse res;
    if (file) {
        std::stringstream buffer;
        buffer << file.rdbuf();
        res.statusCode = 200;
        res.statusText = "OK";
        res.headers["Content-Type"] = "text/html";  // optional: detect mime type
        res.body = buffer.str();
    } else {
        res.statusCode = 404;
        res.statusText = "Not Found";
        res.body = "<html><body><h1>404 Not Found</h1></body></html>";
    }
    return res;
}

HttpResponse GetHandler::handleDirectoryListing(const std::string& dirPath, const std::string& urlPath) const {
    DIR* dir = opendir(dirPath.c_str());
    HttpResponse res;
    if (!dir) {
        res.statusCode = 403;
        res.statusText = "Forbidden";
        res.body = "<html><body><h1>403 Forbidden</h1></body></html>";
        return res;
    }

    std::ostringstream html;
    html << "<html><head><title>Index of " << urlPath << "</title></head><body>";
    html << "<h1>Index of " << urlPath << "</h1><ul>";

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        std::string name = entry->d_name;
        if (name == ".") continue;
        html << "<li><a href=\"" << urlPath << "/" << name << "\">" << name << "</a></li>";
    }
    html << "</ul></body></html>";
    closedir(dir);

    res.statusCode = 200;
    res.statusText = "OK";
    res.body = html.str();
    return res;
}

