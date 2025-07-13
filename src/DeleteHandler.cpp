/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DeleteHandler.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/01 18:29:35 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/13 20:54:52 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "../includes/DeleteHandler.hpp"
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <cstdio>
# include "../includes/Errors.hpp"

DeleteHandler::DeleteHandler() { }

DeleteHandler::~DeleteHandler() { }

HttpResponse DeleteHandler::handle(const HttpRequest &req, const RouteConfig& route, const ServerConfig& serverConfig) const
{
    // !check if redirect can be with delete === STEP 1: Handle redirects (if configured) ===
    if (!route.redirect.empty())
        return createRedirectResponse(route.redirect);
    std::string requestPath = req.uri;
    if (requestPath.find(route.path) == 0)
        requestPath = requestPath.substr(route.path.length());
    std::string filePath = buildFilePath(route.root, requestPath);
    if (!isPathSecure(filePath, route.root)) {
        return createErrorResponse(403, "Forbidden", "Path traversal attempts are not allowed", serverConfig);
    }
    struct stat fileStat;
    if (stat(filePath.c_str(), &fileStat) != 0) {
        return createErrorResponse(404, "Not Found", "The requested file does not exist", serverConfig);
    }
    if (!S_ISREG(fileStat.st_mode)) {
        return createErrorResponse(403, "Forbidden", "Only regular files can be deleted", serverConfig);
    }
    if (access(filePath.c_str(), W_OK) != 0) {
        return createErrorResponse(403, "Forbidden", "Permission denied", serverConfig);
    }
    if (std::remove(filePath.c_str()) == 0) {
        return createSuccessResponse(filePath);
    } else {
        return createErrorResponse(500, "Internal Server Error", "Failed to delete the file", serverConfig);
    }
}

std::string DeleteHandler::buildFilePath(const std::string& root, const std::string& requestPath) const
{
    std::string cleanRoot = root;
    std::string cleanPath = requestPath;
    if (!cleanRoot.empty() && cleanRoot[cleanRoot.length() - 1] == '/') {
        cleanRoot = cleanRoot.substr(0, cleanRoot.length() - 1);
    }
    if (!cleanPath.empty() && cleanPath[0] != '/') {
        cleanPath = "/" + cleanPath;
    }
    return cleanRoot + cleanPath;
}


bool DeleteHandler::isPathSecure(const std::string& filePath, const std::string& rootPath) const
{
    // Check for obvious directory traversal patterns
    if (filePath.find("../") != std::string::npos ||
        filePath.find("..\\") != std::string::npos ||
        filePath.find("/..") != std::string::npos ||
        filePath.find("\\..") != std::string::npos) {
        return false;
    }
    std::string cleanRoot = rootPath;
    if (!cleanRoot.empty() && cleanRoot[cleanRoot.length() - 1] == '/') {
        cleanRoot = cleanRoot.substr(0, cleanRoot.length() - 1);
    }
    return filePath.find(cleanRoot) == 0;
}

HttpResponse DeleteHandler::createSuccessResponse(const std::string& filePath) const
{
    HttpResponse resp;
    resp.statusCode = 200;
    resp.statusText = "OK";
    resp.headers["Content-Type"] = "application/json";

    // Create JSON response
    std::ostringstream json;
    json << "{\n";
    json << "  \"success\": true,\n";
    json << "  \"message\": \"File deleted successfully\",\n";
    json << "  \"file\": \"" << filePath << "\",\n";
    json << "  \"timestamp\": \"" << getCurrentTimestamp() << "\"\n";
    json << "}";
    resp.body = json.str();
    resp.headers["Content-Length"] = std::to_string(resp.body.size());
    return resp;
}

HttpResponse DeleteHandler::createErrorResponse(int statusCode, const std::string& statusText, const std::string& message, const ServerConfig& serverConfig) const
{
    HttpResponse resp;
    resp.statusCode = statusCode;
    resp.statusText = statusText;
    resp.headers["content-type"] = "text/html";
    resp.body = Error::loadErrorPage(resp.statusCode, serverConfig);
    resp.headers["content-length"] = std::to_string(resp.body.size());

    return resp;
}

HttpResponse DeleteHandler::createRedirectResponse(const std::string& redirectUrl) const
{

    HttpResponse resp;
    resp.statusCode = 301;
    resp.statusText = "Moved Permanently";
    resp.headers["Location"] = redirectUrl;
    resp.headers["Content-Type"] = "text/html";

    resp.body = "<!DOCTYPE html>\n<html><head><title>301 Moved Permanently</title></head>";
    resp.body += "<body><h1>301 Moved Permanently</h1>";
    resp.body += "<p>The document has moved <a href=\"" + redirectUrl + "\">here</a>.</p>";
    resp.body += "</body></html>";
    resp.headers["Content-Length"] = std::to_string(resp.body.size());

    return resp;
}

std::string DeleteHandler::getCurrentTimestamp() const
{
    time_t now = time(0);
    char* timeStr = ctime(&now);
    std::string timestamp(timeStr);
    if (!timestamp.empty() && timestamp[timestamp.length() - 1] == '\n') {
        timestamp = timestamp.substr(0, timestamp.length() - 1);
    }
    return timestamp;
}
