/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: baouragh <baouragh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 17:21:08 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/05 14:28:52 by baouragh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "../includes/HttpServer.hpp"
#include <sstream>
#include <algorithm>
#include <cctype> 

HttpRequest::HttpRequest() : method(""), uri(""), version(""), body("") {}

HttpRequest HttpRequest::parse(const std::string& raw)
{
    HttpRequest req;
    std::istringstream stream(raw);
    std::string line;

    // Parse request line
    if (!std::getline(stream, line))
        throw std::runtime_error("Invalid HTTP request: empty request line");

    std::istringstream requestLine(line); // search .exts (.py, .php) // uri --> /
    requestLine >> req.method >> req.uri >> req.version;

    size_t index;
    if ((index = req.uri.find(".py")) || (index = req.uri.find(".php"))) // /path/file.php ([?] --> query) | ([/] --> Path-info --> true)
    {
    //     if (req.uri[index + 1])
    //     {
    //         if (req.uri[index + 1] == '/') // /path/file.php/path?var=value&var1=value
    //         {
                
    //         } // path-info
    //         if (req.uri[index + 1] == '?')
    //         {
                
    //         } // query
    //         if (req.uri[index + 1] != "\0" && req.uri[index + 1] != '?')
    //             throw std::runtime_error("Invalid HTTP request: invalid path");
    //     }
    }

    // Parse headers
    while (std::getline(stream, line)) {
        if (line == "\r" || line.empty()) break;
        size_t colon = line.find(":");
        if (colon == std::string::npos) continue;

        std::string key = line.substr(0, colon);
        std::string value = line.substr(colon + 1);

        // Trim
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t\r") + 1);
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t\r") + 1);

        req.headers[key] = value;
    }

    // Read body if any
    std::getline(stream, req.body, '\0');
    return req;
}

std::string HttpRequest::GetHeader(std::string target) const
{
    std::string value;
        
    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it) {
        std::string current_key = it->first;
        std::transform(current_key.begin(), current_key.end(), current_key.begin(), ::tolower);
        if (current_key == target) 
        {
            value = it->second;
            break;
        }
    }
    return (value);
}
