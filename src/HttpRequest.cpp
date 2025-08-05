/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 17:21:08 by ajabri            #+#    #+#             */
/*   Updated: 2025/08/05 21:15:20 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "../includes/HttpServer.hpp"

HttpRequest::HttpRequest() : method(""), uri(""), version(""), body("") {}

std::string toLower(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    return str;
}

void HttpRequest::parseRequestLine(const std::string& line)
{
    std::istringstream requestLineStream(line);
    requestLineStream >> this->method >> this->uri >> this->version;

    if ((this->method.empty() || this->uri.empty() || this->version.empty())){
        throwHttpError(400, "Invalid HTTP request: info missed in request line");
    }
    else if (this->version != "HTTP/1.1") {
        throwHttpError(505, "HTTP Version Not Supported");
    }
    else if (this->method != "GET" && this->method != "POST" && this->method != "DELETE") {
        throwHttpError(501, "Not Implemented: Unsupported HTTP method");
    }
    else if (this->uri.find_first_not_of("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~:/?#[]@!$&'()*+,;=%") != std::string::npos)
    {
        throwHttpError(400, "Invalid HTTP request: URI contains invalid characters");
    }
    else if (this->uri.size() > 2048)
    {
        throwHttpError(414, "URI Too Long");
    }
}

void HttpRequest::parseHeaderLine(const std::string& line, int& hostFlag)
{
    std::string trimmed_line = line;

    if (!trimmed_line.empty() && trimmed_line[trimmed_line.size() - 1] == '\r') {
        trimmed_line.erase(trimmed_line.size() - 1);
    }

    if (trimmed_line.empty()) {
        if (hostFlag != 1) {
            throwHttpError(400, "Bad request: Host header missing or deplicated");
        }
        return;
    }

    size_t colon = line.find(":");
    if (colon == 0) {
        throwHttpError(400, "Bad request: Empty header key");
    }
    if (colon == std::string::npos) {
        throwHttpError(400, "Bad request: invalid header line (missing colon)");
    }

    std::string key = toLower(line.substr(0, colon));
    std::string value = line.substr(colon + 1);

    if (key[key.length() - 1] == ' ')
        throwHttpError(400, "Bad request: Header key ends with space");
    key.erase(0, key.find_first_not_of(" \t"));
    key.erase(key.find_last_not_of(" \t\r") + 1);
    value.erase(0, value.find_first_not_of(" \t"));
    value.erase(value.find_last_not_of(" \t\r") + 1);

    this->headers[key] = value;
     if (headers.size() > 100 || key.size() > 256 || value.size() > 8192) {
        throwHttpError(431, "Request Header Fields Too Large");
    }

    if (key == "host") {
        if (value.empty()) {
            throwHttpError(400, "Bad request: Host header value empty");
        }
        hostFlag = 1;
    }
}

void HttpRequest::validateAbsoluteUri()
{
    if (this->uri.find("http://", 0) == 0)
    {
        size_t host_start_pos = 7;
        size_t path_start_pos = this->uri.find('/', host_start_pos);
        std::string host_in_uri;

        if (path_start_pos == std::string::npos) {
            host_in_uri = this->uri.substr(host_start_pos);
            this->uri = "/";
        } else {
            host_in_uri = this->uri.substr(host_start_pos, path_start_pos - host_start_pos);
            this->uri = this->uri.substr(path_start_pos);
        }

        std::string host_header = GetHeader("host");
        if (host_header.find(':') != std::string::npos) {
            host_header = host_header.substr(0, host_header.find(':'));
        }

        if (host_in_uri != host_header) {
            throwHttpError(400, "Host in request URI does not match Host header");
        }
    }
}

void HttpRequest::determineBodyProtocol()
{
    if (this->method == "POST"){

        std::string cl_header = GetHeader("content-length");
        std::string te_header = GetHeader("transfer-encoding");

        if ((!cl_header.empty() && !te_header.empty()) || (cl_header.empty() && te_header.empty())) {
            throwHttpError(400, "Bad request: not specified body protocol");
        } else if (!cl_header.empty()) {
            std::stringstream ss(cl_header);
            ss >> this->contentLength;
            if (ss.fail() || this->contentLength < 0) {
                throwHttpError(400, "Invalid Content-Length header value");
            }
            this->isChunked = false;
        } else if (!te_header.empty() && te_header == "chunked") {
            this->isChunked = true;
            this->contentLength = 0;
        }
        else if (!te_header.empty() && te_header != "chunked") {
            throwHttpError(501, "Not Implemented: Unsupported Transfer-Encoding");
        }
    }
    else {
        this->isChunked = false;
        this->contentLength = 0;
    }
}

void HttpRequest::parseHeaders(const std::string& rawHeaders)
{
    std::istringstream stream(rawHeaders);
    std::string line;
    int hostFlag = 0;

    if (!std::getline(stream, line) || line.empty()){
        throwHttpError(400, "Invalid HTTP request: empty request line");
    }
    parseRequestLine(line);

    while (std::getline(stream, line)) {
        if (!line.empty() && line[line.size() - 1] == '\r') {
        line.erase(line.size() - 1);
        }

        if (line.empty()) {
            if (hostFlag != 1) {
                throwHttpError(400, "Bad request: Host header missing or deplicated");
            }
            break;
        }

        parseHeaderLine(line, hostFlag);
    }

    validateAbsoluteUri();

    determineBodyProtocol();
}

bool HttpRequest::parseBody(std::string& connectionBuffer, unsigned long long maxBodySize) {
    if (!isChunked && contentLength == 0) {
        return true;
    }

    if (!isChunked) {
        if (contentLength > maxBodySize)
            throwHttpError(413, "request entity Too Large");
        if (contentLength > 0 && connectionBuffer.length() >= contentLength) {
            this->body.append(connectionBuffer.substr(0, contentLength));
            connectionBuffer.erase(0, contentLength);
            return true;
        }
        return false;
    } else {
        return decodeChunked(connectionBuffer, this->body, maxBodySize);
    }
}

bool HttpRequest::parseChunkSize(const std::string& sizeHex, unsigned long long& chunkSize)
{
    if (sizeHex.empty()) {
        return false;
    }

    std::string hexPart = sizeHex;
    size_t semicolon = hexPart.find(';');
    if (semicolon != std::string::npos) {
        hexPart = hexPart.substr(0, semicolon);
    }
    for (size_t i = 0; i < hexPart.size(); ++i) {
        char c = hexPart[i];
        if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))) {
            return false;
        }
    }
    std::istringstream iss(hexPart);
    iss >> std::hex >> chunkSize;

    if (iss.fail() || !iss.eof()) {
        return false;
    }

    return true;
}

bool HttpRequest::skipTrailerHeaders(std::string& buffer, size_t startPos)
{
    size_t pos = startPos;

    while (true) {
        size_t lineEnd = buffer.find("\r\n", pos);
        if (lineEnd == std::string::npos) {
            return false;
        }

        if (lineEnd == pos) {
            buffer.erase(0, pos + 2);
            return true;
        }
        pos = lineEnd + 2;
    }
}

bool HttpRequest::decodeChunked(std::string& buffer, std::string& decodedOutput, unsigned long long maxBodySize) {
    while (true)
    {
        size_t sizeEndPos = buffer.find("\r\n");
        if (sizeEndPos == std::string::npos) {
            return false;
        }
        std::string sizeHex = buffer.substr(0, sizeEndPos);
        unsigned long long chunkSize;
        if (!parseChunkSize(sizeHex, chunkSize)) {
            throwHttpError(400, "Invalid chunk size format");
        }
        if (chunkSize > 0 && (decodedOutput.length() + chunkSize) > maxBodySize) {
            throwHttpError(413, "Payload Too Large");
        }
        if (chunkSize == 0) {
            size_t trailerStart = sizeEndPos + 2;
            if (!skipTrailerHeaders(buffer, trailerStart)) {
                return false;
            }
            return true;
        }
        size_t dataStartPos = sizeEndPos + 2;
        size_t chunkTotalLength = dataStartPos + chunkSize + 2;
        if (buffer.length() < chunkTotalLength) {
            return false;
        }

        if (buffer.substr(dataStartPos + chunkSize, 2) != "\r\n") {
            throwHttpError(400, "Invalid chunk format: missing trailing CRLF");
        }
        decodedOutput.append(buffer, dataStartPos, chunkSize);
        buffer.erase(0, chunkTotalLength);
    }
}


void HttpRequest::throwHttpError(int statusCode, const std::string& message) {
    throw HttpException(statusCode, message);
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

HttpRequest::HttpException::HttpException(int code, const std::string& msg)
    : statusCode(code), message(msg) {
}

HttpRequest::HttpException::~HttpException() throw() {
}

const char* HttpRequest::HttpException::what() const throw() {
    return message.c_str();
}

int HttpRequest::HttpException::getStatusCode() const {
    return statusCode;
}