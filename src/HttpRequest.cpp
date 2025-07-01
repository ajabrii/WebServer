/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 17:21:08 by ajabri            #+#    #+#             */
/*   Updated: 2025/06/26 17:27:36 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "../includes/HttpServer.hpp"

HttpRequest::HttpRequest() : method(""), uri(""), version(""), body("") {}

HttpRequest HttpRequest::parse(const std::string& raw) {
    HttpRequest req;
    std::istringstream stream(raw);
    std::string line;

    // Parse request line
    if (!std::getline(stream, line))
        throw std::runtime_error("Invalid HTTP request: empty request line");

    std::istringstream requestLine(line);
    requestLine >> req.method >> req.uri >> req.version;

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
