/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DeleteHandler.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/01 18:29:35 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/23 15:18:31 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/DeleteHandler.hpp"
#include "../includes/Utils.hpp"
#include "../includes/ResponseBuilder.hpp"
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <cstdio>

DeleteHandler::DeleteHandler() {}

DeleteHandler::~DeleteHandler() {}

HttpResponse DeleteHandler::handle(const HttpRequest &req, const RouteConfig &route, const ServerConfig &serverConfig) const
{
    std::string requestPath = req.uri;
    if (requestPath.find(route.path) == 0)
        requestPath = requestPath.substr(route.path.length());
    std::string filePath = buildFilePath(route.root, requestPath);
    if (!isPathSecure(filePath, route.root))
    {
        return ResponseBuilder::createErrorResponse(403, "Forbidden", serverConfig);
    }
    struct stat fileStat;
    if (stat(filePath.c_str(), &fileStat) != 0)
    {
        return ResponseBuilder::createErrorResponse(404, "Not Found", serverConfig);
    }
    if (!S_ISREG(fileStat.st_mode))
    {
        return ResponseBuilder::createErrorResponse(403, "Forbidden", serverConfig);
    }
    if (access(filePath.c_str(), W_OK) != 0)
    {
        return ResponseBuilder::createErrorResponse(403, "Forbidden", serverConfig);
    }
    if (std::remove(filePath.c_str()) == 0)
    {
        return ResponseBuilder::createDeleteSuccessResponse(filePath);
    }
    else
    {
        return ResponseBuilder::createErrorResponse(500, "Internal Server Error", serverConfig);
    }
}

std::string DeleteHandler::buildFilePath(const std::string &root, const std::string &requestPath) const
{
    std::string cleanRoot = root;
    std::string cleanPath = requestPath;
    if (!cleanRoot.empty() && cleanRoot[cleanRoot.length() - 1] == '/')
    {
        cleanRoot = cleanRoot.substr(0, cleanRoot.length() - 1);
    }
    if (!cleanPath.empty() && cleanPath[0] != '/')
    {
        cleanPath = "/" + cleanPath;
    }
    return cleanRoot + cleanPath;
}

bool DeleteHandler::isPathSecure(const std::string &filePath, const std::string &rootPath) const
{
    // Check for obvious directory traversal patterns
    if (filePath.find("../") != std::string::npos ||
        filePath.find("/..") != std::string::npos)
    {
        return false;
    }
    std::string cleanRoot = rootPath;
    if (!cleanRoot.empty() && cleanRoot[cleanRoot.length() - 1] == '/')
    {
        cleanRoot = cleanRoot.substr(0, cleanRoot.length() - 1);
    }
    return filePath.find(cleanRoot) == 0;
}
