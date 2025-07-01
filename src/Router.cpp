/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/30 09:02:04 by ajabri            #+#    #+#             */
/*   Updated: 2025/06/30 11:21:45 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Router.hpp"
# include <iostream>
# include <string>

std::string clean_line(std::string line)
{
    // Trim leading and trailing whitespace
    line.erase(0, line.find_first_not_of(" \t\n\r"));
    line.erase(line.find_last_not_of(" \t\n\r") + 1);

    // Remove trailing semicolon if it exists
    if (!line.empty() && line.back() == ';')
    {
        line.pop_back();
    }
    return line;
}


const RouteConfig* Router::match(const HttpRequest& request, const ServerConfig& serverConfig) const
{
    std::cout << "Matching route..." << std::endl;
    const RouteConfig* match = 0;
    const RouteConfig* defaultRoute = 0;
    
    for (size_t j = 0; j < serverConfig.routes.size(); ++j) {
        const std::string& routePath = serverConfig.routes[j].path;

        if (!defaultRoute || routePath.length() < defaultRoute->path.length()) {
            defaultRoute = &serverConfig.routes[j];
        }

        if (request.uri.find(routePath) == 0) {
            if (!match || routePath.length() > match->path.length()) {
                match = &serverConfig.routes[j];
            }
        }
    }

    if (!match) {
        match = defaultRoute;
    }

    std::cout << "Matched route: " << (match ? match->path : "none") << std::endl;
    return match;
}


// const RouteConfig* Router::match(HttpRequest& request, ServerConfig serverConfig, int fd) const
// {
//     // std::cout << "Matching route..." << std::endl;
//     // RouteConfig* match = 0;
//     // RouteConfig* defaultRoute =  0;
    
//     // for (size_t j = 0; j < serverConfig.routes.size(); ++j) {
//     //     const std::string& routePath = serverConfig.routes[j].path;
    
//     //     // Save the shortest route as fallback (often "/")
//     //     if (!defaultRoute || routePath.length() < defaultRoute->path.length()) {
//     //         defaultRoute = &serverConfig.routes[j];
//     //     }
    
//     //     // Check if routePath is a prefix of request.path
//     //     if (request.uri.find(routePath) == 0) {
//     //         // Longest prefix match
//     //         if (!match || routePath.length() > match->path.length()) {
//     //             match = &serverConfig.routes[j];
//     //         }
//     //     }
//     // }
    
//     // // Use longest matching prefix or fallback to shortest route
//     // if (!match) {
//     //         match = defaultRoute;
//     // }
        
//     //     // Handle redirect if any
//     //     // if (!match->redirect.empty()) {
//     //     //     std::string redirect_response = "HTTP/1.1 301 Moved Permanently\r\nLocation: " + match->redirect + "\r\n\r\n";
//     //     //     send(fd, redirect_response.c_str(), redirect_response.size(), 0);
//     //     //     return;
//     //     // }
//     //     std::cout << "-----------match: " << match->path << std::endl;

//     // std::string filePath = clean_line(match->root) + clean_line(request.uri);
//     // filePath = filePath.substr(0, filePath.length() - 1); // Remove trailing slash if exists
//     // std::cout << "request path: " << request.uri << std::endl;
//     // std::cout << "match root: " << match->root << std::endl;
//     // std::cout << "File path: " << filePath << std::endl;
//     // std::cout << "match->directory_listing: " << match->directory_listing << std::endl;

//     // // if (match->directory_listing) {
//     // //     std::string listing = DirectoryListing(filePath, request.path);
//     // //     std::string response = "HTTP/1.1 403 Forbidden \r\nContent-Type: text/html\r\n\r\n";
//     // //     response += listing;
//     // //     send(fd, response.c_str(),  response.size(), 0);
//     // // } else {
//     //     std::ifstream file(filePath.c_str());
//     //     if (file) {
//     //         std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
//     //         std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
//     //         response += content;
//     //         send(fd, response.c_str(), response.size(), 0);
//     //     } else {
//     //         std::cerr << "File not found: " << filePath << std::endl;
//     //         std::string error_response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n";
//     //         error_response += "<html><body><h1>404 Not Found</h1></body></html>";
//     //         send(fd, error_response.c_str(), error_response.size(), 0);
//     //     }
//     //     return 0; 
// }

