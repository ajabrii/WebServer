/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ytarhoua <ytarhoua@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 17:21:08 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/03 18:38:27 by ytarhoua         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "../includes/HttpServer.hpp"

HttpRequest::HttpRequest() : method(""), uri(""), version(""), body("") {}

HttpRequest HttpRequest::parse(const std::string& raw)
{
    HttpRequest req;
    std::istringstream stream(raw);
    std::string line;

    // Parse request line
    if (!std::getline(stream, line)){
        req.status = 400; // bad request
        throw std::runtime_error("Invalid HTTP request: empty request line");
    }

    std::istringstream requestLine(line);
    requestLine >> req.method >> req.uri >> req.version;
    if ((req.method.empty() || req.uri.empty() || req.version.empty())){
        req.status = 400;
    }
    else if (req.version != "HTTP/1.1")
        req.status = 505; // 505 http Version Not Supported

    // Parse headers
    while (std::getline(stream, line)) {
        int hostFlag = 0;
        if (line == "\r" || line.empty()){ // for no or more then one host
            if (hostFlag != 1)
                req.status = 400;
          break;
        }
        size_t colon = line.find(":");
        if (colon == 0)
            req.status = 400; //bad one;
        if (colon == std::string::npos) continue;

        std::string key = line.substr(0, colon);
        std::string value = line.substr(colon + 1);
        if (key == "Host")
        {
            if (value.empty())
                req.status = 400;
            hostFlag = 1;
        }
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
