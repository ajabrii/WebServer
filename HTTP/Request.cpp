/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kali <kali@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/07 15:35:16 by kali              #+#    #+#             */
/*   Updated: 2025/05/07 16:21:30 by kali             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"



void Request::parseHttpRequest()
{

    std::string rawRequest = this->requesto;

    // std:: cout << "rawRequest: " << rawRequest << std::endl;
    Request req;
    size_t headerEnd = rawRequest.find("\r\n\r\n");
    // std::cout << "headerEnd:----------------------------------------> " << headerEnd << std::endl;

    std::string headerSection = rawRequest.substr(0, headerEnd);
    // std::cout << "headerSection:----------------------------------------> " << headerSection << std::endl;
    std::string bodySection = rawRequest.substr(headerEnd + 4);

    std::istringstream stream(headerSection);
    std::string line;

    // First line: method, path, version
    std::getline(stream, line);
    std::istringstream requestLine(line);
    requestLine >> req.method >> req.path >> req.version;
    this->path = req.path;
    this->method = req.method;
    this->version = req.version;

    // *Remaining lines: headers
    while (std::getline(stream, line)) {
        if (line.back() == '\r')
            line.pop_back(); // remove \r

        size_t colon = line.find(":");
        if (colon != std::string::npos) {
            std::string key = line.substr(0, colon);
            std::string value = line.substr(colon + 1);
            value = value;
            req.headers[key] = value;
        }
    }

    this->body = bodySection;
    this->headers = req.headers;
}

