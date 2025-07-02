/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestDispatcher.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/30 09:14:07 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/02 11:46:52 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "../includes/RequestDispatcher.hpp"
#include "../includes/GetHandler.hpp"
#include "../includes/DeleteHandler.hpp"
#include "../includes/PostHandler.hpp"
# include <iostream>
# include <fcntl.h>
#include <dirent.h>
#include <sstream>
# include <fstream>
std::string clean_line(std::string line);




HttpResponse RequestDispatcher::dispatch(const HttpRequest& req, const RouteConfig& route) const
{
    
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
    if (req.method == "GET")
    {
        // Getha
        return GetHandler().handle(req, route);
    }
    else if (req.method == "POST")
    {
        // PostHandler postHandler;
        return PostHandler().handle(req, route);
    }
    else if (req.method == "DELETE")
    {
        // DeleteHandler deleteHandler;
        return DeleteHandler().handle(req, route);
    }
    else
    {
        HttpResponse res;
        res.statusCode = 501; // Not Implemented
        res.statusText = "Not Implemented";
        res.body = "HTTP method not implemented.";
        return res;
    }
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