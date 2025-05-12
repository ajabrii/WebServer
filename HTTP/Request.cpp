/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/07 15:35:16 by kali              #+#    #+#             */
/*   Updated: 2025/05/12 16:41:11 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"
#include <fstream>


void SubHost(Request &req, std::string value)
{
    size_t pos = value.find(":");

    std::string port = value.substr(pos + 1);
    std::string add = value.substr(0, pos);

    if (pos != std::string::npos) {
        req.headers["Port"] = port;
        req.headers["Address"] = add;
    } else {
        req.headers["Host"] = value;
    }
}

void Request::parseHttpRequest()
{


    std::string rawRequest = this->requesto;
    Request req;
    size_t headerEnd = rawRequest.find("\r\n\r\n");
    std::string headerSection = rawRequest.substr(0, headerEnd);
    std::string bodySection = rawRequest.substr(headerEnd + 4);

    std::istringstream stream(headerSection);
    std::string line;

    // First line: method, path, version
    // std::cout << "----------------------------------------> bef`" << this->hasCgi << "'\n";
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
            if (key == "Host")
            {
                SubHost(req, value);
                continue;
            }
            req.headers[key] = value;
        }
    }

    // for (std::map<std::string, std::string>::iterator it = req.headers.begin(); it != req.headers.end(); ++it) {
    //     std::cout << "Header: " << it->first << ": " << it->second << std::endl;
    // }
    // exit(0);
    this->body = bodySection;
    this->headers = req.headers;
}


void Request::sendResponse(int fd, const std::string &response)
{
    send(fd, response.c_str(), response.size(), 0);
}

void Request::generateResponse(int fd)
{
    std::string response;

    // Define the file path (you can later set this dynamically from this->path)
    // std::string path = "index.html";

    // Try opening the file
    // std::ifstream file(path.c_str());
    // if (!file.is_open())
    // {
    //     std::cerr << "Error opening file: " << path << std::endl;
    //     // Send 404 response
    //     response = this->version + " 404 Not Found\r\n";
    //     response += "Content-Type: text/plain\r\n";
    //     response += "Content-Length: 13\r\n";
    //     response += "\r\n";
    //     response += "404 Not Found\n";
    //     sendResponse(fd, response);
    //     return;
    // }

    // Read file contents
    // std::string fileContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    // file.close();

    // Build the response
    response += this->version + " 200 OK\r\n";
    response += "Content-Type: text/html\r\n";  // You can later improve this using file extension
    response += "Content-Length: 14" "\r\n";
    response += "\r\n"; // End of headers
    response += "hello world\r\n"; // Body
    response += "\r\n"; // End of body
    response += "\r\n"; // End of body


    // Send the response
    sendResponse(fd, response);
}
