/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GetHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/01 18:20:34 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/12 18:03:18 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/GetHandler.hpp"
#include "../includes/RequestDispatcher.hpp"
#include <iostream>
#include <fcntl.h>
#include <dirent.h>
#include <sstream>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>
#include <cctype>
# include "../includes/Errors.hpp"
GetHandler::GetHandler() { }

GetHandler::~GetHandler() { }

/**
 * Main handler for GET requests
 * Processes the request according to this priority:
 * 1. Redirects (highest priority)
 * 2. File serving or directory listing based on path
 * 3. Error responses for not found/forbidden
 */

 //
HttpResponse GetHandler::handle(const HttpRequest &req, const RouteConfig& route, const ServerConfig& serverConfig) const
{
    std::cout << "\033[1;34m[GET Handler]\033[0m Processing request for URI: " << req.uri << std::endl;
    
    // *DONE=== STEP 1: Handle redirects (highest priority) ===
    if (!route.redirect.empty()) {
        std::cout << "\033[1;33m[GET Handler]\033[0m Redirect configured to: " << route.redirect << std::endl;
        return handleRedirect(route.redirect);
    }

    // *DONE === STEP 2: Build the file system path ===
    std::string requestPath = req.uri;
    
    // Remove the route path prefix from the request URI to get the relative path
    // Example: route.path="/images", req.uri="/images/photo.jpg" -> requestPath="/photo.jpg"
    if (requestPath.find(route.path) == 0) {
        requestPath = requestPath.substr(route.path.length());
        std::cout << "\033[1;36m[GET Handler]\033[0m Stripped route path, remaining: '" << requestPath << "'" << std::endl;
    }
    
    // Normalize paths: ensure root doesn't end with '/' and requestPath starts with '/'
    std::string cleanRoot = route.root;
    if (!cleanRoot.empty() && cleanRoot[cleanRoot.length() - 1] == '/')
        cleanRoot = cleanRoot.substr(0, cleanRoot.length() - 1);
    if (!requestPath.empty() && requestPath[0] != '/')
        requestPath = "/" + requestPath;
    // Build final file system path
    std::string filePath = cleanRoot + requestPath;
    //! To remove this print statement later on
    std::cout << "\033[1;36m[GET Handler]\033[0m Path construction:" << std::endl;
    std::cout << "  - Route path: '" << route.path << "'" << std::endl;
    std::cout << "  - Route root: '" << route.root << "'" << std::endl;
    std::cout << "  - Request URI: '" << req.uri << "'" << std::endl;
    std::cout << "  - Final path: '" << filePath << "'" << std::endl;

    // === STEP 3: Check what exists at the path ===
    struct stat pathStat;
    if (stat(filePath.c_str(), &pathStat) == 0) {
        // Path exists, check if it's a directory or file
        if (S_ISDIR(pathStat.st_mode)) {
            std::cout << "\033[1;32m[GET Handler]\033[0m Path is a directory, directory_listing = " << (route.directory_listing ? "true" : "false") << std::endl;
            return handleDirectory(filePath, req.uri, route.directory_listing, serverConfig);
        } else if (S_ISREG(pathStat.st_mode)) {
            std::cout << "\033[1;32m[GET Handler]\033[0m Path is a regular file" << std::endl;
            return serveStaticFile(filePath, serverConfig);
        } else {
            std::cout << "\033[1;31m[GET Handler]\033[0m Path exists but is not a file or directory" << std::endl;
        }
    } else {
        std::cout << "\033[1;31m[GET Handler]\033[0m Path does not exist: " << filePath << std::endl;
    }
    
    // === STEP 4: Return 404 for not found ===
    return createNotFoundResponse(serverConfig);
}

/**
 * Handles directory requests
 * If directory listing is enabled, generates an HTML index
 * Otherwise, tries to serve index.html or returns 403 Forbidden
 */
HttpResponse GetHandler::handleDirectory(const std::string& dirPath, const std::string& urlPath, bool listingEnabled, const ServerConfig& serverConfig) const
{
    if (listingEnabled) {
        std::cout << "\033[1;32m[GET Handler]\033[0m Directory listing enabled, generating index" << std::endl;
        return handleDirectoryListing(dirPath, urlPath, serverConfig);
    } else {
        std::cout << "\033[1;33m[GET Handler]\033[0m Directory listing disabled, looking for index file" << std::endl;
        
        // Try to serve index.html from the directory
        std::string indexPath = dirPath + "/index.html";
        struct stat indexStat;
        
        if (stat(indexPath.c_str(), &indexStat) == 0 && S_ISREG(indexStat.st_mode)) {
            std::cout << "\033[1;32m[GET Handler]\033[0m Found index.html, serving it" << std::endl;
            return serveStaticFile(indexPath, serverConfig);
        } else {
            std::cout << "\033[1;31m[GET Handler]\033[0m No index.html found, returning 403 Forbidden" << std::endl;
            return createForbiddenResponse(serverConfig);
        }
    }
}

/**
 * Handles HTTP redirects
 * Returns a 301 Moved Permanently response with Location header
 */
HttpResponse GetHandler::handleRedirect(const std::string& redirectUrl) const
{
    std::cout << "\033[1;33m[GET Handler]\033[0m Creating redirect response to: " << redirectUrl << std::endl;
    
    HttpResponse res;
    res.statusCode = 301;
    res.statusText = "Moved Permanently";
    res.headers["Location"] = redirectUrl;
    res.headers["Content-Type"] = "text/html";
    
    // Create a user-friendly HTML redirect page //!(machi darouri)
    // res.body = "<!DOCTYPE html>\n<html><head><title>301 Moved Permanently</title></head>";
    // res.body += "<body><h1>301 Moved Permanently</h1>";
    // res.body += "<p>The document has moved <a href=\"" + redirectUrl + "\">here</a>.</p>";
    // res.body += "</body></html>";
    res.headers["Content-Length"] = std::to_string(res.body.size());
    
    return res;
}

/**
 * Serves static files from the file system
 * Automatically detects MIME type based on file extension
 * Handles binary files correctly
 */
HttpResponse GetHandler::serveStaticFile(const std::string& filePath, const ServerConfig& serverConfig) const 
{
    std::cout << "\033[1;32m[GET Handler]\033[0m Attempting to serve file: '" << filePath << "'" << std::endl;
    
    // Open file in binary mode to handle all file types correctly
    std::ifstream file(filePath.c_str(), std::ios::binary);//? ios::binary tels the stream to read the file as binay exactly as it is in disk
    HttpResponse res;
    
    if (file.is_open()) {
        // Read entire file content
        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();
        
        // Build successful response
        res.statusCode = 200;
        res.statusText = "OK";
        res.body = buffer.str();
        
        // Set appropriate Content-Type based on file extension
        std::string contentType = getMimeType(filePath);
        res.headers["Content-Type"] = contentType;
        res.headers["Content-Length"] = std::to_string(res.body.size());
        
        std::cout << "\033[1;32m[GET Handler]\033[0m Successfully served file:" << std::endl;
        std::cout << "  - Size: " << res.body.size() << " bytes" << std::endl;
        std::cout << "  - MIME type: " << contentType << std::endl;
    } else {
        std::cout << "\033[1;31m[GET Handler]\033[0m Failed to open file: " << filePath << std::endl;
        res = createNotFoundResponse(serverConfig);
    }
    
    return res;
}

/**
 * Generates HTML directory listing
 * Creates a table with file names and types
 * Includes parent directory navigation
 */
HttpResponse GetHandler::handleDirectoryListing(const std::string& dirPath, const std::string& urlPath, const ServerConfig& serverConfig) const
{
    std::cout << "\033[1;32m[GET Handler]\033[0m Generating directory listing for: " << dirPath << std::endl;
    
    DIR* dir = opendir(dirPath.c_str());
    if (!dir) {
        std::cout << "\033[1;31m[GET Handler]\033[0m Cannot open directory: " << dirPath << std::endl;
        return createForbiddenResponse(serverConfig);
    }

    // Build HTML page with CSS styling
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

    // Add parent directory link if not at root (machi darouri)
    // if (urlPath != "/" && urlPath != "") {
    //     std::string parentPath = getParentPath(urlPath);
    //     std::cout << "\033[1;33m[GET Handler]\033[0m Adding parent directory link: " << parentPath << std::endl;
    //     html << "<tr><td><span class='icon'>📁</span><a href=\"" << parentPath << "\">../</a></td><td>Directory</td></tr>";
    // }

    // List directory contents
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL)
    {
        std::string name = entry->d_name;
        if (name == "." || name == "..")
            continue;
        std::string fullPath = dirPath + "/" + name;
        std::string linkPath = buildLinkPath(urlPath, name);
        
        // Determine file type and icon
        struct stat entryStat;
        std::string type = "File";
        std::string icon = "📄";
        
        if (stat(fullPath.c_str(), &entryStat) == 0) {
            if (S_ISDIR(entryStat.st_mode)) {
                type = "Directory";
                icon = "📁";
                if (linkPath[linkPath.length() - 1] != '/') {
                    linkPath += "/";
                }
            }
        }
        
        html << "<tr><td><span class='icon'>" << icon << "</span><a href=\"" << linkPath << "\">" << name << "</a></td>";
        html << "<td>" << type << "</td></tr>";
    }
    
    html << "</table></div><p>return to the <a href=\"/\">homepage</a>.</p></body></html>";
    closedir(dir);

    HttpResponse res;
    res.statusCode = 200;
    res.statusText = "OK";
    res.headers["Content-Type"] = "text/html; charset=utf-8";
    res.body = html.str();
    res.headers["Content-Length"] = std::to_string(res.body.size());
    
    std::cout << "\033[1;32m[GET Handler]\033[0m Generated directory listing (" << res.body.size() << " bytes)" << std::endl;
    return res;
}

/**
 * Determines MIME type based on file extension
 * Returns appropriate Content-Type header value
 */
std::string GetHandler::getMimeType(const std::string& filePath) const
{
    size_t dotPos = filePath.find_last_of('.');
    if (dotPos == std::string::npos) {
        return "application/octet-stream"; // Default for files without extension
    }
    
    std::string extension = filePath.substr(dotPos + 1);
    
    // Convert extension to lowercase for comparison
    for (size_t i = 0; i < extension.length(); ++i) {
        extension[i] = std::tolower(extension[i]);
    }
    
    // Common MIME type mappings
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
    if (extension == "xml") return "application/xml";
    if (extension == "pdf") return "application/pdf";
    if (extension == "zip") return "application/zip";
    if (extension == "mp3") return "audio/mpeg";
    if (extension == "mp4") return "video/mp4";
    if (extension == "avi") return "video/x-msvideo";
    if (extension == "webm") return "video/webm";
    if (extension == "wav") return "audio/wav";
    if (extension == "ogg") return "audio/ogg";
    
    return "application/octet-stream"; // Default for unknown types
}

/**
 * !remove this function later is not important
 * Helper function to get parent directory path
 * Handles URL path normalization
 */
// std::string GetHandler::getParentPath(const std::string& urlPath) const
// {
//     std::string parentPath = urlPath;
    
//     // Remove trailing slash if present
//     if (parentPath[parentPath.length() - 1] == '/') {
//         parentPath = parentPath.substr(0, parentPath.length() - 1);
//     }
    
//     // Find last slash and cut there
//     size_t lastSlash = parentPath.find_last_of('/');
//     if (lastSlash != std::string::npos) {
//         parentPath = parentPath.substr(0, lastSlash + 1);
//     } else {
//         parentPath = "/";
//     }
    
//     return parentPath;
// }

/**
 * Helper function to build proper link paths for directory listing
 * Ensures correct URL formatting
 */
std::string GetHandler::buildLinkPath(const std::string& urlPath, const std::string& name) const
{
    std::string linkPath = urlPath;
    
    // Ensure urlPath ends with slash
    if (linkPath[linkPath.length() - 1] != '/') {
        linkPath += "/";
    }
    
    linkPath += name;
    return linkPath;
}

/**
 * Creates a standardized 404 Not Found response
 * Uses custom error page if configured, otherwise uses default HTML
 */
HttpResponse GetHandler::createNotFoundResponse(const ServerConfig& serverConfig) const {
    return createErrorResponse(404, "Not Found", serverConfig);
}

/**
 * Creates a standardized 403 Forbidden response
 * Uses custom error page if configured, otherwise uses default HTML
 */
HttpResponse GetHandler::createForbiddenResponse(const ServerConfig& serverConfig) const {
    return createErrorResponse(403, "Forbidden", serverConfig);
}

/**
 * Creates error responses using custom error pages when available
 * Falls back to default HTML pages when custom pages are not configured
 */
HttpResponse GetHandler::createErrorResponse(int statusCode, const std::string& statusText, const ServerConfig& serverConfig) const
{
    std::cout << "\033[1;31m[GET Handler]\033[0m Creating " << statusCode << " error response" << std::endl;
    
    // Check if custom error page is configured
    std::map<int, std::string>::const_iterator it = serverConfig.error_pages.find(statusCode);
    if (it != serverConfig.error_pages.end()) {
        std::cout << "\033[1;33m[GET Handler]\033[0m Using custom error page: " << it->second << std::endl;
        return loadCustomErrorPage(statusCode, it->second);
    } else {
        std::cout << "\033[1;33m[GET Handler]\033[0m Using default error page" << std::endl;
        // Create default error page
        HttpResponse res;
        res.statusCode = statusCode;
        res.statusText = statusText;
        res.headers["Content-Type"] = "text/html";
        
        // Create a nice default error page
        res.body = "<!DOCTYPE html>\n<html><head><title>" + std::to_string(statusCode) + " " + statusText + "</title>";
        res.body += "<style>body{font-family:Arial,sans-serif;text-align:center;padding:50px;background-color:#f5f5f5;}";
        res.body += ".error-container{background-color:white;padding:40px;border-radius:8px;box-shadow:0 2px 10px rgba(0,0,0,0.1);display:inline-block;}";
        res.body += "h1{color:#dc3545;margin-bottom:20px;}p{color:#666;margin-bottom:30px;}";
        res.body += ".error-code{font-size:72px;font-weight:bold;color:#dc3545;margin-bottom:20px;}</style></head>";
        res.body += "<body><div class='error-container'>";
        res.body += "<div class='error-code'>" + std::to_string(statusCode) + "</div>";
        res.body += "<h1>" + statusText + "</h1>";
        
        if (statusCode == 404) {
            res.body += "<p>The requested resource could not be found on this server.</p>";
        } else if (statusCode == 403) {
            res.body += "<p>You don't have permission to access this resource.</p>";
        } else {
            res.body += "<p>An error occurred while processing your request.</p>";
        }
        
        res.body += "</div></body></html>";
        res.headers["Content-Length"] = std::to_string(res.body.size());
        return res;
    }
}

/**
 * Loads custom error page from filesystem
 * Returns default error page if custom page cannot be loaded
 */
HttpResponse GetHandler::loadCustomErrorPage(int statusCode, const std::string& errorPagePath) const
{
    std::cout << "\033[1;33m[GET Handler]\033[0m Loading custom error page: " << errorPagePath << std::endl;
    
    std::ifstream file(errorPagePath.c_str());
    HttpResponse res;
    
    if (file.is_open()) {
        // Successfully opened custom error page
        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();
        
        res.statusCode = statusCode;
        res.statusText = (statusCode == 404) ? "Not Found" : 
                        (statusCode == 403) ? "Forbidden" : 
                        (statusCode == 500) ? "Internal Server Error" : "Error";
        res.headers["Content-Type"] = "text/html";
        res.body = buffer.str();
        res.headers["Content-Length"] = std::to_string(res.body.size());
        
        std::cout << "\033[1;32m[GET Handler]\033[0m Successfully loaded custom error page (" << res.body.size() << " bytes)" << std::endl;
    } else {
        std::cout << "\033[1;31m[GET Handler]\033[0m Failed to load custom error page: " << errorPagePath << std::endl;
        std::cout << "\033[1;33m[GET Handler]\033[0m Falling back to default error page" << std::endl;
        
        // Fall back to default error page
        res.statusCode = statusCode;
        res.statusText = (statusCode == 404) ? "Not Found" : 
                        (statusCode == 403) ? "Forbidden" : 
                        (statusCode == 500) ? "Internal Server Error" : "Error";
        res.headers["Content-Type"] = "text/html";
        res.body = "<!DOCTYPE html>\n<html><head><title>" + std::to_string(statusCode) + " " + res.statusText + "</title></head>";
        res.body += "<body><h1>" + std::to_string(statusCode) + " " + res.statusText + "</h1>";
        res.body += "<p>Custom error page could not be loaded.</p></body></html>";
        res.headers["Content-Length"] = std::to_string(res.body.size());
    }
    
    return res;
}

/**
 * Legacy handler method for backward compatibility
 * Uses default error pages when ServerConfig is not available// ! no need for it  for now
 */
// HttpResponse GetHandler::handle(const HttpRequest &req, const RouteConfig& route) const
// {
//     // Create a default ServerConfig for backward compatibility
//     ServerConfig defaultConfig;
//     return handle(req, route, defaultConfig);
// }

