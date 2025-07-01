/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestDispatcher.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ytarhoua <ytarhoua@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/30 09:14:07 by ajabri            #+#    #+#             */
/*   Updated: 2025/06/30 16:12:52 by ytarhoua         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "../includes/RequestDispatcher.hpp"
# include <iostream>
# include <fcntl.h>
#include <dirent.h>
#include <sstream>
# include <fstream>
std::string clean_line(std::string line);




HttpResponse RequestDispatcher::dispatch(const HttpRequest& req, const RouteConfig& route) const
{
    
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

HttpResponse RequestDispatcher::handleRedirect(const std::string& redirectUrl) const {
    HttpResponse res;
    res.statusCode = 301;
    res.statusText = "Moved Permanently";
    res.headers["Location"] = redirectUrl;
    res.body = "<html><body><h1>301 Moved Permanently</h1></body></html>";
    return res;
}

HttpResponse RequestDispatcher::serveStaticFile(std::string& filePath) const 
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

HttpResponse RequestDispatcher::handleDirectoryListing(const std::string& dirPath, const std::string& urlPath) const {
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


// HttpResponse RequestDispatcher::dispatch(const HttpRequest& request, const RouteConfig& route) const
// {
//     HttpResponse response;

//     std:: cout << route.path << std::endl; // Accessing route path for potential logging or processing
//     // Here we would typically call the appropriate handler based on the method
//     // For now, let's assume we have a GetHandler that handles GET requests
//     if (request.method == "GET") {
//         // GetHandler getHandler;
//         // response = getHandler.handle(request, route);
//         response.statusCode = 200; // Placeholder for successful GET response
//         response.body = "GET request handled successfully.";
//     } else {
//         response.statusCode = 405; // Method Not Allowed
//         response.body = "Method not allowed.";
//     }

//     return response;
// }


// HttpResponse RequestDispatcher::servStaticFile(const std::string& filepath_uri) const
// {
//     HttpResponse response;
  
//     return response;
// }

// HttpResponse RequestDispatcher::DirectoryListing(const std::string& path, const std::string& urlPath) const
// {
//     HttpResponse response;
//     DIR* dir = opendir(path.c_str());
//     if (!dir) {
//         response.body = "<html><body><h1>403 Forbidden</h1></body></html>";
//         return response; // or use your error page
//     }

//     std::ostringstream html;
//     html << "<html><head><title>Index of " << urlPath << "</title></head><body>";
//     html << "<h1>Index of " << urlPath << "</h1><ul>";

//     struct dirent* entry;
//     while ((entry = readdir(dir)) != NULL) {
//         std::string name = entry->d_name;
//         if (name == ".") continue;
//         std::string link = urlPath + (urlPath.back() == '/' ? "" : "/") + name;
//         html << "<li><a href=\"" << link << "\">" << name << "</a></li>";
//     }

//     html << "</ul></body></html>";
//     response.body = html.str();
//     response.statusCode = 200; // OK
//     response.statusText = "OK";
//     response.headers["Content-Type"] = "text/html";
//     response.headers["Content-Length"] = std::to_string(response.body.size());
//     closedir(dir);

//     return response;
// }

// HttpResponse RequestDispatcher::handleRedirect(const std::string& redirectUri) const
// {
//     HttpResponse response;
//     response.statusCode = 301; // Moved Permanently
//     response.statusText = "Moved Permanently";
//     response.headers["Location"] = redirectUri;
//     response.body = "Redirecting to " + redirectUri;
//     response.headers["Content-Type"] = "text/plain";
//     response.headers["Content-Length"] = std::to_string(response.body.size());
    
//     return response;
// }