/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GetHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: youness <youness@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/01 18:20:34 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/11 23:09:19 by youness          ###   ########.fr       */
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
#include <sys/stat.h>

std::string clean_line(std::string line);
// HttpResponse RequestDispatcher::handleRedirect(const std::string &redirectUrl) const;

GetHandler::GetHandler(){ }

GetHandler::~GetHandler() {}

HttpResponse GetHandler::handle(const HttpRequest &req, const RouteConfig& route) const
{
    // 1. Redirects still have the highest priority
    if (!route.redirect.empty()) {
        return handleRedirect(route.redirect);
    }

    // --- PATH LOGIC ---
    std::string requestPath = req.uri;
    if (requestPath.find(route.path) == 0) {
        requestPath.erase(0, route.path.length());
    }
    std::string filePath = route.root + requestPath;
    // --- END PATH LOGIC ---

    // --- DECISION LOGIC ---
    struct stat file_info;
    // Check if the path exists and if it's a directory
    if (stat(filePath.c_str(), &file_info) == 0 && S_ISDIR(file_info.st_mode))
    {
        // Path is a directory.
        // Check if we should serve a default index file.
        if (!route.indexFile.empty())
        {
            return handleDirectoryIndex(filePath, route);
        }
        // Otherwise, check if we should show a directory listing.
        else if (route.directory_listing)
        {
            return handleDirectoryListing(filePath, req.uri);
        }
    }
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

HttpResponse GetHandler::handleDirectoryIndex(const std::string& dirPath, const RouteConfig& route) const
{
    if (route.indexFile.empty()) {
        HttpResponse res;
        res.statusCode = 403;
        res.statusText = "Forbidden";
        res.body = "<html><body><h1>403 Forbidden</h1><p>No index file configured.</p></body></html>";

        std::ostringstream oss;
        oss << res.body.length();
        res.headers["Content-Length"] = oss.str();
        return res;
    }

    std::string indexPath = dirPath;
    if (indexPath[indexPath.length() - 1] != '/') {
        indexPath += "/";
    }
    indexPath += route.indexFile;

    return serveStaticFile(indexPath);
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

