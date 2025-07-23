/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GetHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/01 18:20:34 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/23 15:13:23 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/GetHandler.hpp"
#include "../includes/RequestDispatcher.hpp"
#include "../includes/Utils.hpp"
#include "../includes/ResponseBuilder.hpp"
#include <iostream>
#include <fcntl.h>
#include <dirent.h>
#include <sstream>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>
#include <cctype>
#include "../includes/Errors.hpp"

GetHandler::GetHandler() {}

GetHandler::~GetHandler() {}

HttpResponse GetHandler::handle(const HttpRequest &req, const RouteConfig &route, const ServerConfig &serverConfig) const
{
    std::cout << URI_PROCESS_LOG << req.uri << std::endl;
    if (!route.redirect.empty())
        return handleRedirect(route.redirect);
    std::string requestPath = extractPath(req.uri);
    requestPath = normalizePath(requestPath);
    if (requestPath.find(route.path) == 0)
        requestPath = requestPath.substr(route.path.length());
    std::string cleanRoot = route.root;
    if (!cleanRoot.empty() && cleanRoot[cleanRoot.length() - 1] == '/')
        cleanRoot = cleanRoot.substr(0, cleanRoot.length() - 1);
    if (!requestPath.empty() && requestPath[0] != '/')
        requestPath = "/" + requestPath;
    std::string filePath = cleanRoot + requestPath;
    struct stat pathStat;
    if (stat(filePath.c_str(), &pathStat) == 0)
    {
        if (S_ISDIR(pathStat.st_mode))
            return handleDirectory(filePath, req.uri, route.directory_listing, serverConfig, route.indexFile);
        else if (S_ISREG(pathStat.st_mode))
            return serveStaticFile(filePath, serverConfig);
    }
    return createNotFoundResponse(serverConfig);
}

HttpResponse GetHandler::handleDirectory(const std::string &dirPath, const std::string &urlPath, bool listingEnabled, const ServerConfig &serverConfig, const std::string &indexFile) const
{
    if (indexFile.empty() && listingEnabled)
        return handleDirectoryListing(dirPath, urlPath, serverConfig);
    else
    {
        std::string indexPath = dirPath + "/" + indexFile;
        struct stat indexStat;
        if (stat(indexPath.c_str(), &indexStat) == 0 && S_ISREG(indexStat.st_mode))
            return serveStaticFile(indexPath, serverConfig);
    }
    return createForbiddenResponse(serverConfig);
}

HttpResponse GetHandler::handleRedirect(const std::string &redirectUrl) const
{
    std::cout << REDIRCT_LOG << redirectUrl << std::endl;
    return ResponseBuilder::createRedirectResponse(301, "Moved Permanently", redirectUrl);
}

HttpResponse GetHandler::serveStaticFile(const std::string &filePath, const ServerConfig &serverConfig) const
{
    std::cout << STATIC_FILE_LOG << filePath << "'" << std::endl;
    std::ifstream file(filePath.c_str(), std::ios::binary);
    HttpResponse res;

    if (file.is_open())
    {
        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();
        res.statusCode = 200;
        res.statusText = "OK";
        res.body = buffer.str();
        std::string contentType = getMimeType(filePath);
        res.headers["content-type"] = contentType;
        res.headers["content-length"] = Utils::toString(res.body.size());
    }
    else
        res = createNotFoundResponse(serverConfig);
    return res;
}

HttpResponse GetHandler::handleDirectoryListing(const std::string &dirPath, const std::string &urlPath, const ServerConfig &serverConfig) const
{
    std::cout << DIRECTORY_LISTING_LOG << dirPath << std::endl;
    DIR *dir = opendir(dirPath.c_str());
    if (!dir)
        return createForbiddenResponse(serverConfig);
    std::ostringstream html;
    html << "<!DOCTYPE html>\n";
    html << "<html><head><title>Index of " << urlPath << "</title>";
    html << "<style>";
    html << "body{font-family:Arial,sans-serif;margin:40px;background-color:#f5f5f5;}";
    html << ".container{background-color:white;padding:20px;border-radius:8px;box-shadow:0 2px 4px rgba(0,0,0,0.1);}";
    html << "h1{color:#333;border-bottom:2px solid #007bff;padding-bottom:10px;}";
    html << "table{border-collapse:collapse;width:100%;margin-top:20px;}";
    html << "th,td{text-align:left;padding:12px;border-bottom:1px solid #ddd;}";
    html << "th{background-color:#007bff;color:white;}";
    html << "tr:hover{background-color:#f9f9f9;}";
    html << "a{color:#007bff;text-decoration:none;}";
    html << "a:hover{text-decoration:underline;}";
    html << ".icon{margin-right:8px;}";
    html << "</style></head><body>";
    html << "<div class='container'>";
    html << "<h1>📁 Index of " << urlPath << "</h1>";
    html << "<table><tr><th>Name</th><th>Type</th></tr>";
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        std::string name = entry->d_name;
        if (name == "." || name == "..")
            continue;
        std::string fullPath = dirPath + "/" + name;
        std::string linkPath = buildLinkPath(urlPath, name);
        struct stat entryStat;
        std::string type = "File";
        std::string icon = "📄";

        if (stat(fullPath.c_str(), &entryStat) == 0)
        {
            if (S_ISDIR(entryStat.st_mode))
            {
                type = "Directory";
                icon = "📁";
                if (linkPath[linkPath.length() - 1] != '/')
                {
                    linkPath += "/";
                }
            }
        }
        html << "<tr><td><span class='icon'>" << icon << "</span><a href=\"" << linkPath << "\">" << name << "</a></td>";
        html << "<td>" << type << "</td></tr>";
    }

    html << "</table></div><p style=\"text-align:center;\">return to the <a href=\"/\">homepage</a>.</p></body></html>";
    closedir(dir);

    HttpResponse res;
    res.statusCode = 200;
    res.statusText = "OK";
    res.headers["content-type"] = "text/html; charset=utf-8";
    res.body = html.str();
    res.headers["content-length"] = Utils::toString(res.body.size());
    return res;
}

std::string GetHandler::getMimeType(const std::string &filePath) const
{
    size_t dotPos = filePath.find_last_of('.');
    if (dotPos == std::string::npos)
    {
        return "application/octet-stream";
    }

    std::string extension = filePath.substr(dotPos + 1);

    for (size_t i = 0; i < extension.length(); ++i)
    {
        extension[i] = std::tolower(extension[i]);
    }

    if (extension == "html" || extension == "htm")
        return "text/html";
    if (extension == "css")
        return "text/css";
    if (extension == "js")
        return "application/javascript";
    if (extension == "json")
        return "application/json";
    if (extension == "png")
        return "image/png";
    if (extension == "jpg" || extension == "jpeg")
        return "image/jpeg";
    if (extension == "gif")
        return "image/gif";
    if (extension == "svg")
        return "image/svg+xml";
    if (extension == "ico")
        return "image/x-icon";
    if (extension == "txt")
        return "text/plain";
    if (extension == "xml")
        return "application/xml";
    if (extension == "pdf")
        return "application/pdf";
    if (extension == "zip")
        return "application/zip";
    if (extension == "mp3")
        return "audio/mpeg";
    if (extension == "mp4")
        return "video/mp4";
    if (extension == "avi")
        return "video/x-msvideo";
    if (extension == "webm")
        return "video/webm";
    if (extension == "wav")
        return "audio/wav";
    if (extension == "ogg")
        return "audio/ogg";

    return "application/octet-stream";
}

std::string GetHandler::buildLinkPath(const std::string &urlPath, const std::string &name) const
{
    std::string linkPath = urlPath;

    if (linkPath[linkPath.length() - 1] != '/')
    {
        linkPath += "/";
    }

    linkPath += name;
    return linkPath;
}

HttpResponse GetHandler::createNotFoundResponse(const ServerConfig &serverConfig) const
{
    return createErrorResponse(404, "Not Found", serverConfig);
}

HttpResponse GetHandler::createForbiddenResponse(const ServerConfig &serverConfig) const
{
    return createErrorResponse(403, "Forbidden", serverConfig);
}

HttpResponse GetHandler::createErrorResponse(int statusCode, const std::string &statusText, const ServerConfig &serverConfig) const
{
    std::cout << "\033[1;31m[GET Handler]\033[0m Creating " << statusCode << " error response" << std::endl;
    std::cout << "\033[1;33m[GET Handler]\033[0m Using default error page" << std::endl;

    HttpResponse res;
    res.statusCode = statusCode;
    res.statusText = statusText;
    res.headers["content-type"] = "text/html";
    res.body = Error::loadErrorPage(statusCode, serverConfig);        // Load body first
    res.headers["content-length"] = Utils::toString(res.body.size()); // Then set content-length
    return res;
}

std::string GetHandler::extractPath(const std::string &uri) const
{

    size_t queryPos = uri.find('?');
    size_t fragmentPos = uri.find('#');

    size_t endPos = std::string::npos;
    if (queryPos != std::string::npos && fragmentPos != std::string::npos)
        endPos = std::min(queryPos, fragmentPos);
    else if (queryPos != std::string::npos)
        endPos = queryPos;
    else if (fragmentPos != std::string::npos)
        endPos = fragmentPos;

    if (endPos == std::string::npos)
        return uri;
    return uri.substr(0, endPos);
}

std::string GetHandler::normalizePath(const std::string &path) const
{
    // PATH NORMALIZATION: Clean up the path for security and consistency
    // This function resolves relative path components and removes redundancies
    // Examples:
    //   "/path//to/./file" -> "/path/to/file"
    //   "/path/to/../other/file" -> "/path/other/file"
    //   "/path/to/../../.." -> "/"

    std::string normalized = path;

    // STEP 1: Replace multiple consecutive slashes with single slash
    // This handles cases like "///" or "/path//to//file"
    size_t pos = 0;
    while ((pos = normalized.find("//", pos)) != std::string::npos)
    {
        normalized.replace(pos, 2, "/");
    }

    // STEP 2: Handle . and .. path components
    // Split the path into components and process each one
    std::vector<std::string> components;
    std::istringstream stream(normalized);
    std::string component;

    // Split by '/' delimiter
    while (std::getline(stream, component, '/'))
    {
        if (component.empty() || component == ".")
        {
            continue;
        }
        else if (component == "..")
        {
            if (!components.empty())
            {
                components.pop_back();
            }
            // If components is empty, we're trying to go above root
            // Ignore this (can't go above root directory)
        }
        else
        {
            // Regular path component - add it to our list
            components.push_back(component);
        }
    }

    // STEP 3: Rebuild the normalized path
    std::string result = "/"; // Always start with root
    for (size_t i = 0; i < components.size(); ++i)
    {
        if (i > 0)
            result += "/"; // Add separator between components
        result += components[i];
    }

    // STEP 4: Handle special case where original path was just "/"
    if (components.empty() && !path.empty() && path[0] == '/')
    {
        return "/";
    }

    return result;
}
