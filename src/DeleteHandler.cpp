/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DeleteHandler.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/01 18:29:35 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/12 10:55:32 by ajabri           ###   ########.fr       */
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

DeleteHandler::DeleteHandler() { }

DeleteHandler::~DeleteHandler() { }

/**
 * Legacy DELETE handler method (for backward compatibility)
 * Delegates to the new method with a default ServerConfig
 */
HttpResponse DeleteHandler::handle(const HttpRequest &req, const RouteConfig& route) const
{
    // Create a minimal ServerConfig for error handling
    ServerConfig defaultConfig;
    return handle(req, route, defaultConfig);
}

/**
 * Main DELETE handler method with ServerConfig support
 * Handles file deletion with proper security checks and error handling
 */
HttpResponse DeleteHandler::handle(const HttpRequest &req, const RouteConfig& route, const ServerConfig& serverConfig) const
{
    std::cout << "\033[1;31m[DELETE Handler]\033[0m Processing DELETE request for URI: " << req.uri << std::endl;
    
    // !check if redirect can be with delete === STEP 1: Handle redirects (if configured) ===
    if (!route.redirect.empty()) {
        std::cout << "\033[1;33m[DELETE Handler]\033[0m Redirect configured to: " << route.redirect << std::endl;
        return createRedirectResponse(route.redirect, serverConfig);
    }
    
    // === STEP 2: Build and validate the file path ===
    std::string requestPath = req.uri;
    
    // Remove route path prefix from request URI
    if (requestPath.find(route.path) == 0) {
        requestPath = requestPath.substr(route.path.length());
        std::cout << "\033[1;36m[DELETE Handler]\033[0m Stripped route path, remaining: '" << requestPath << "'" << std::endl;
    }
    
    // Build file path
    std::string filePath = buildFilePath(route.root, requestPath);
    
    std::cout << "\033[1;36m[DELETE Handler]\033[0m Path construction:" << std::endl;
    std::cout << "  - Route path: '" << route.path << "'" << std::endl;
    std::cout << "  - Route root: '" << route.root << "'" << std::endl;
    std::cout << "  - Request URI: '" << req.uri << "'" << std::endl;
    std::cout << "  - Final path: '" << filePath << "'" << std::endl;
    
    // === STEP 3: Security validation ===
    if (!isPathSecure(filePath, route.root)) {
        std::cout << "\033[1;31m[DELETE Handler]\033[0m Security violation: Path traversal attempt detected" << std::endl;
        return createErrorResponse(403, "Forbidden", "Path traversal attempts are not allowed", serverConfig);
    }
    
    // === STEP 4: Check if file exists and is deletable ===
    struct stat fileStat;
    if (stat(filePath.c_str(), &fileStat) != 0) {
        std::cout << "\033[1;31m[DELETE Handler]\033[0m File does not exist: " << filePath << std::endl;
        return createErrorResponse(404, "Not Found", "The requested file does not exist", serverConfig);
    }
    
    // Check if it's a regular file (not a directory or special file)
    if (!S_ISREG(fileStat.st_mode)) {
        std::cout << "\033[1;31m[DELETE Handler]\033[0m Target is not a regular file: " << filePath << std::endl;
        return createErrorResponse(403, "Forbidden", "Only regular files can be deleted", serverConfig);
    }
    
    // === STEP 5: Check write permissions ===
    if (access(filePath.c_str(), W_OK) != 0) {
        std::cout << "\033[1;31m[DELETE Handler]\033[0m No write permission for file: " << filePath << std::endl;
        return createErrorResponse(403, "Forbidden", "Permission denied", serverConfig);
    }
    
    // === STEP 6: Attempt to delete the file ===
    std::cout << "\033[1;33m[DELETE Handler]\033[0m Attempting to delete file: " << filePath << std::endl;
    
    if (std::remove(filePath.c_str()) == 0) {
        std::cout << "\033[1;32m[DELETE Handler]\033[0m Successfully deleted file: " << filePath << std::endl;
        return createSuccessResponse(filePath, serverConfig);
    } else {
        std::cout << "\033[1;31m[DELETE Handler]\033[0m Failed to delete file: " << filePath << std::endl;
        return createErrorResponse(500, "Internal Server Error", "Failed to delete the file", serverConfig); // use error pages if available
    }
}

/**
 * Builds a secure file path from root and request path
 * Handles path normalization and prevents directory traversal
 */
std::string DeleteHandler::buildFilePath(const std::string& root, const std::string& requestPath) const
{
    std::string cleanRoot = root;
    std::string cleanPath = requestPath;
    
    // Remove trailing slash from root if present
    if (!cleanRoot.empty() && cleanRoot[cleanRoot.length() - 1] == '/') {
        cleanRoot = cleanRoot.substr(0, cleanRoot.length() - 1);
    }
    
    // Ensure request path starts with /
    if (!cleanPath.empty() && cleanPath[0] != '/') {
        cleanPath = "/" + cleanPath;
    }
    
    return cleanRoot + cleanPath;
}

/**
 * Validates that the file path is secure and doesn't contain directory traversal
 * Returns true if path is safe, false otherwise
 */
bool DeleteHandler::isPathSecure(const std::string& filePath, const std::string& rootPath) const {
    // Check for obvious directory traversal patterns
    if (filePath.find("../") != std::string::npos || 
        filePath.find("..\\") != std::string::npos ||
        filePath.find("/..") != std::string::npos ||
        filePath.find("\\..") != std::string::npos) {
        return false;
    }
    
    // Additional security: ensure the resolved path is within the root directory
    // This is a simplified check - in production, you'd want to use realpath()
    std::string cleanRoot = rootPath;
    if (!cleanRoot.empty() && cleanRoot[cleanRoot.length() - 1] == '/') {
        cleanRoot = cleanRoot.substr(0, cleanRoot.length() - 1);
    }
    
    return filePath.find(cleanRoot) == 0;
}

/**
 * Creates a success response for successful file deletion
 */
HttpResponse DeleteHandler::createSuccessResponse(const std::string& filePath, const ServerConfig& serverConfig) const {
    (void)serverConfig; // Suppress unused parameter warning for now
    
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

/**
 * Creates error responses with proper HTTP status codes
 * Uses custom error pages if available in ServerConfig
 */
HttpResponse DeleteHandler::createErrorResponse(int statusCode, const std::string& statusText, 
                                               const std::string& message, const ServerConfig& serverConfig) const {
    HttpResponse resp;
    resp.statusCode = statusCode;
    resp.statusText = statusText;
    
    // Check if custom error page is configured
    std::map<int, std::string>::const_iterator it = serverConfig.error_pages.find(statusCode);
    if (it != serverConfig.error_pages.end()) {
        // Try to load custom error page
        std::ifstream file(it->second.c_str());
        if (file.is_open()) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            file.close();
            
            resp.headers["Content-Type"] = "text/html";
            resp.body = buffer.str();
            resp.headers["Content-Length"] = std::to_string(resp.body.size());
            
            std::cout << "\033[1;33m[DELETE Handler]\033[0m Using custom error page: " << it->second << std::endl;
            return resp;
        }
    }
    
    // Use default error response
    resp.headers["Content-Type"] = "application/json";
    
    std::ostringstream json;
    json << "{\n";
    json << "  \"success\": false,\n";
    json << "  \"error\": {\n";
    json << "    \"code\": " << statusCode << ",\n";
    json << "    \"message\": \"" << statusText << "\",\n";
    json << "    \"details\": \"" << message << "\"\n";
    json << "  },\n";
    json << "  \"timestamp\": \"" << getCurrentTimestamp() << "\"\n";
    json << "}";
    
    resp.body = json.str();
    resp.headers["Content-Length"] = std::to_string(resp.body.size());
    
    return resp;
}

/**
 * Creates a redirect response
 */
HttpResponse DeleteHandler::createRedirectResponse(const std::string& redirectUrl, const ServerConfig& serverConfig) const {
    (void)serverConfig; // Suppress unused parameter warning
    
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

/**
 * Gets current timestamp for logging and response purposes
 */
std::string DeleteHandler::getCurrentTimestamp() const {
    time_t now = time(0);
    char* timeStr = ctime(&now);
    std::string timestamp(timeStr);
    // Remove trailing newline
    if (!timestamp.empty() && timestamp[timestamp.length() - 1] == '\n') {
        timestamp = timestamp.substr(0, timestamp.length() - 1);
    }
    return timestamp;
}
