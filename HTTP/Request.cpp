/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: baouragh <baouragh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/07 15:35:16 by kali              #+#    #+#             */
/*   Updated: 2025/06/12 18:50:27 by baouragh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"
#include <fstream>

std::string Request::exts[N_EXTENTION] = {".py", ".php", ".cpp"};


bool isValidQueryString(const std::string& qs) {
    size_t pos = 0;
    while (pos < qs.length()) {
        size_t eq = qs.find('=', pos);
        size_t amp = qs.find('&', pos);

        if (eq == std::string::npos || (amp != std::string::npos && eq > amp))
            return false;

        if (eq == pos) // empty key
            return false;

        pos = (amp == std::string::npos) ? qs.length() : amp + 1;
    }
    return true;
}


void Request::check_cgi()
{
    size_t pos;
    std::string tmp;
    bool is_query = false;

    for (size_t i = 0; i < N_EXTENTION; i++)
    {
        
        pos = this->path.find(exts[i]);
        if (pos == std::string::npos)
            continue;
        tmp = this->path.substr(pos);
        std::cout << "----------------------------------------> `" << tmp << "'\n";
        std::cout << "------------------tmp.substr(0, exts[i].size())----------------------> `" << tmp.substr(0, exts[i].size()) << "'\n";
        std::cout << "-----------------------tmp.c_str()[exts[i].size()]-----------------> `" << tmp.c_str()[exts[i].size()] << "'\n";

        if (tmp == exts[i] || (is_query = (tmp.substr(0, exts[i].size()) == exts[i] && tmp.c_str()[exts[i].size()] == '?')))
        {
            if (is_query)
            {
                
                if (!(this->hasQurey = isValidQueryString(tmp.substr(exts[i].size()))))
                std::cerr << "Query is invalid\n";
                else
                {
                    this->query = tmp.substr(exts[i].size());
                    std::cerr << "Query is valid : " << this->query << "\n";
                    this->script_path = this->path.substr(0, this->path.find('?'));
                }
            }
            else
                this->script_path = tmp;
            this->hasCgi = true;
        }
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
    std::cout << "----------------------------------------> bef`" << this->hasCgi << "'\n";
    std::getline(stream, line);
    std::istringstream requestLine(line);
    requestLine >> req.method >> req.path >> req.version;
    this->path = req.path;
    this->check_cgi();// .find(.php) 
    std::cout << "----------------------------------------> `" << this->hasCgi << "'\n";
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
            // value = value;
            req.headers[key] = value;
        }
    }

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
