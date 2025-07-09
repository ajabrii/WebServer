/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 17:21:08 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/09 08:30:15 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "../includes/HttpServer.hpp"

HttpRequest::HttpRequest() : method(""), uri(""), version(""), body("") {}

HttpRequest HttpRequest::parse(const std::string& raw)
{
    HttpRequest req;
    std::istringstream stream(raw);
    std::string line;
    int hostFlag = 0;

    // Parse request line
    if (!std::getline(stream, line)){
        throwHttpError(400, "Invalid HTTP request: empty request line");
    }

    std::istringstream requestLine(line);
    requestLine >> req.method >> req.uri >> req.version;
    if ((req.method.empty() || req.uri.empty() || req.version.empty())){
        throwHttpError(400, "Invalid HTTP request: empty request line");
    }
    else if (req.version != "HTTP/1.1")
        throwHttpError(505, "http Version Not Supported"); // 505 http Version Not Supported //!don't forget leaks !!!

    // Parse headers
    while (std::getline(stream, line)) {
        if (line == "\r" || line.empty()){ // for no or more then one host
            if (hostFlag != 1)
                throwHttpError(400, "bad request, Host");
          break;
        }
        size_t colon = line.find(":");
        if (colon == 0)
            throwHttpError(400, "bad request."); //bad one;
        if (colon == std::string::npos) {
            throwHttpError(400, "bad request, Host");
            // continue;
        }

        std::string key = line.substr(0, colon);
        std::string value = line.substr(colon + 1);
        if (key == "Host")
        {
            if (value.empty())
                throwHttpError(400, "bad request, Host error");
            hostFlag = 1;
        }
        // Trim
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t\r") + 1);
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t\r") + 1);

        req.headers[key] = value;
    }

    if (!req.headers["Content-Length"].empty() && !req.headers["Transfer-Encoding"].empty())
    throwHttpError(400, "bad request"); // Both are set → HTTP spec forbids this
    
    else if (!req.headers["Content-Length"].empty()){
        std::string all_body;

        std::string cl = req.headers["Content-Length"];
        if (!cl.empty()) {
            std::stringstream ss(cl);
            ss >> req.contentLength;
            if (ss.fail()) {
                throwHttpError(400, "Invalid Content-Length");
            }
        } else {
            req.contentLength = 0;
        }
        std::cout << "size is ::" << req.contentLength << std::endl;
        std::cout << "all_body is ::" << all_body << std::endl;
        std::getline(stream, all_body, '\0');
        if (all_body.size() < req.contentLength)
        throwHttpError(400, "bad request, incomplete body"); // incomplete body
        else {
            req.bodyReceived = all_body.size();
            
            if (req.bodyReceived >= req.contentLength) {
                req.body = all_body.substr(0, req.contentLength);
                all_body.erase(0, req.contentLength);
            }
        }
    }
    else if (!req.headers["Transfer-Encoding"].empty()) {
        std::cout << "heeeeeeeeeeeeeeeeeere" << std::endl;
        std::string all_body;
        std::getline(stream, all_body, '\0');
        req.body = decodeChunked(all_body);
    }
    else {
        // no body content?
    }
    return req;
}

std::string HttpRequest::decodeChunked(const std::string& chunkedBody) {
    std::istringstream stream(chunkedBody);
    std::string decoded;
    std::string line;

    while (std::getline(stream, line)) {
        // Remove trailing \r because getline stops at \n only.
        if (!line.empty() && line[line.size() - 1] == '\r')
            line.erase(line.size() - 1);

        // Convert hex size line to int
        std::stringstream ss(line);
        int chunkSize = 0;
        ss >> std::hex >> chunkSize;

        if (ss.fail()) {
            throwHttpError(400, "Invalid chunk size");
        }
        if (chunkSize == 0) {
            break; // end
        }

        // Read chunk data
        char *buffer = new char[chunkSize];
        stream.read(buffer, chunkSize);
        if (stream.gcount() != chunkSize) {
            delete[] buffer;
            throwHttpError(400, "Chunk too short");
        }

        decoded.append(buffer, chunkSize);
        delete[] buffer;

        // Next 2 bytes must be \r\n
        char cr, lf;
        stream.get(cr);
        stream.get(lf);
        if (cr != '\r' || lf != '\n') {
            throwHttpError(400, "Expected CRLF after chunk");
        }
    }
    return decoded;
}



void HttpRequest::throwHttpError(int statusCode, const std::string& message) {
    std::ostringstream oss;
    oss << statusCode;
    throw std::runtime_error("HTTP error " + oss.str() + ": " + message);

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