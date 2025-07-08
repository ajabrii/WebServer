/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ytarhoua <ytarhoua@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 17:21:08 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/08 18:14:36 by ytarhoua         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "../includes/HttpServer.hpp"

HttpRequest::HttpRequest() : method(""), uri(""), version(""), body("") {}

// HttpRequest HttpRequest::parseHeaders(const std::string& raw)
// {
//     HttpRequest req;
//     std::istringstream stream(raw);
//     std::string line;
//     int hostFlag = 0;

//     // Parse request line
//     if (!std::getline(stream, line)){
//         throwHttpError(400, "Invalid HTTP request: empty request line");
//     }

//     std::istringstream requestLine(line);
//     requestLine >> req.method >> req.uri >> req.version;
//     if ((req.method.empty() || req.uri.empty() || req.version.empty())){
//         throwHttpError(400, "Invalid HTTP request: empty request line");
//     }
//     else if (req.version != "HTTP/1.1")
//         throwHttpError(505, "http Version Not Supported"); // 505 http Version Not Supported //!don't forget leaks !!!

//     // Parse headers
//     while (std::getline(stream, line)) {
//         if (line == "\r" || line.empty()){ // for no or more then one host
//             if (hostFlag != 1)
//                 throwHttpError(400, "bad request, Host");
//           break;
//         }
//         size_t colon = line.find(":");
//         if (colon == 0)
//             throwHttpError(400, "bad request."); //bad one;
//         if (colon == std::string::npos) {
//             throwHttpError(400, "bad request, Host");
//             // continue;
//         }

//         std::string key = line.substr(0, colon);
//         std::string value = line.substr(colon + 1);
//         if (key == "Host")
//         {
//             if (value.empty())
//                 throwHttpError(400, "bad request, Host error");
//             hostFlag = 1;
//         }
//         // Trim
//         key.erase(0, key.find_first_not_of(" \t"));
//         key.erase(key.find_last_not_of(" \t\r") + 1);
//         value.erase(0, value.find_first_not_of(" \t"));
//         value.erase(value.find_last_not_of(" \t\r") + 1);

//         req.headers[key] = value;
//     }

//     if (!req.headers["Content-Length"].empty() && !req.headers["Transfer-Encoding"].empty())
//     throwHttpError(400, "bad request"); // Both are set → HTTP spec forbids this
    
//     else if (!req.headers["Content-Length"].empty()){
//         std::string all_body;

//         std::string cl = req.headers["Content-Length"];
//         if (!cl.empty()) {
//             std::stringstream ss(cl);
//             ss >> req.contentLength;
//             if (ss.fail()) {
//                 throwHttpError(400, "Invalid Content-Length");
//             }
//         } else {
//             req.contentLength = 0;
//         }
//         std::cout << "len is ::" << req.contentLength << std::endl;
//         std::getline(stream, all_body, '\0');
//         std::cout << "recived size is ::" << all_body.size() << std::endl;
//         if (all_body.size() < req.contentLength)
//         throwHttpError(400, "bad request, incomplete body"); // incomplete body
//         else {
//             req.bodyReceived = all_body.size();
            
//             if (req.bodyReceived >= req.contentLength) {
//                 req.body = all_body.substr(0, req.contentLength);
//                 all_body.erase(0, req.contentLength);
//             }
//         }
//     }
//     else if (!req.headers["Transfer-Encoding"].empty()) {
//         // std::cout << "heeeeeeeeeeeeeeeeeere" << std::endl;
//         std::string all_body;
//         std::getline(stream, all_body, '\0');
//         req.body = decodeChunked(all_body);
//     }
//     else {
//         // no body content?
//     }
//     return req;
// }

void HttpRequest::parseHeaders(const std::string& rawHeaders)
{
    std::istringstream stream(rawHeaders);
    std::string line;
    int hostFlag = 0;

    // Parse request line
    if (!std::getline(stream, line)){
        throwHttpError(400, "Invalid HTTP request: empty request line");
    }

    std::istringstream requestLineStream(line); // Renamed to avoid conflict with outer 'stream'
    requestLineStream >> this->method >> this->uri >> this->version; // Set members of 'this' object
    if ((this->method.empty() || this->uri.empty() || this->version.empty())){
        throwHttpError(400, "Invalid HTTP request: empty request line");
    }
    else if (this->version != "HTTP/1.1") {
        throwHttpError(505, "HTTP Version Not Supported");
    }

    // Parse headers
    while (std::getline(stream, line)) {
        // The loop will naturally end when it runs out of lines from 'rawHeaders'
        // which should already be up to the \r\n\r\n boundary.
        // So, the check for "\r" or empty line is not strictly needed here if rawHeaders is clean.
        // However, keeping a trimmed check for empty lines might be safer for malformed headers.
        std::string trimmed_line = line;
        if (!trimmed_line.empty() && trimmed_line[trimmed_line.size() - 1] == '\r') {
            trimmed_line.erase(trimmed_line.size() - 1);
        }
        if (trimmed_line.empty()) { // End of headers
            if (hostFlag != 1) { // Check if Host header was found
                throwHttpError(400, "Bad request: Host header missing or malformed");
            }
            break; // Exit loop, headers are done
        }

        size_t colon = line.find(":");
        if (colon == 0) { // Header key cannot be empty
            throwHttpError(400, "Bad request: Empty header key");
        }
        if (colon == std::string::npos) { // Header line without a colon
            // This is generally a bad request. You could continue or throw.
            throwHttpError(400, "Bad request: Malformed header line (missing colon)");
        }

        std::string key = line.substr(0, colon);
        std::string value = line.substr(colon + 1);

        // Trim key and value (as you had it)
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t\r") + 1);
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t\r") + 1);

        // Store header (using this->headers)
        this->headers[key] = value;

        // Check for Host header
        if (key == "Host") {
            if (value.empty()) {
                throwHttpError(400, "Bad request: Host header value empty");
            }
            hostFlag = 1;
        }
    }

    // After parsing all headers, determine body expectations
    std::string cl_header = GetHeader("content-length"); // Use GetHeader for case-insensitivity
    std::string te_header = GetHeader("transfer-encoding"); // Use GetHeader for case-insensitivity

    if (!cl_header.empty() && !te_header.empty()) {
        throwHttpError(400, "Bad request: Both Content-Length and Transfer-Encoding headers are present");
    } else if (!cl_header.empty()) {
        std::stringstream ss(cl_header);
        ss >> this->contentLength; // Set member contentLength
        if (ss.fail() || this->contentLength < 0) {
            throwHttpError(400, "Invalid Content-Length header value");
        }
        this->isChunked = false; // Not chunked
    } else if (!te_header.empty() && te_header == "chunked") {
        this->isChunked = true; // Is chunked
        this->contentLength = 0; // Content-Length is irrelevant for chunked
    } else {
        // No body expected (GET, HEAD, or other methods without body)
        this->contentLength = 0;
        this->isChunked = false;
    }
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