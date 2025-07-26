/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DeleteHandler.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/01 18:29:35 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/26 11:02:08 by ajabri           ###   ########.fr       */
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
#include <cstdlib>

DeleteHandler::DeleteHandler() {}

DeleteHandler::~DeleteHandler() {}

HttpResponse DeleteHandler::handle(const HttpRequest &req, const RouteConfig &route, const ServerConfig &serverConfig) const
{
    std::cout << "\033[1;31m[DELETE] request uri to delete :" << req.uri << std::endl;
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

    // URL decode the path to handle encoded characters like %20 (space)
    cleanPath = urlDecode(cleanPath);

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

std::string DeleteHandler::urlDecode(const std::string &encoded) const
{
    std::string decoded;

    for (size_t i = 0; i < encoded.length(); ++i)
    {
        if (encoded[i] == '%' && i + 2 < encoded.length())
        {
            // Get the two hex digits after %
            std::string hexStr = encoded.substr(i + 1, 2);
            char *endPtr;
            long int value = std::strtol(hexStr.c_str(), &endPtr, 16);

            // Check if conversion was successful and value is valid
            if (endPtr == hexStr.c_str() + 2 && value >= 0 && value <= 255)
            {
                decoded += static_cast<char>(value);
                i += 2; // Skip the two hex digits
            }
            else
            {
                // Invalid hex sequence, keep the % as is
                decoded += encoded[i];
            }
        }
        else if (encoded[i] == '+')
        {
            decoded += ' ';
        }
        else
        {
            decoded += encoded[i];
        }
    }

    return decoded;
}
