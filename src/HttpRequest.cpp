/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: baouragh <baouragh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 17:21:08 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/16 09:26:44 by baouragh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "../includes/HttpServer.hpp"

HttpRequest::HttpRequest() : method(""), uri(""), version(""), body("") {}

std::string toLower(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    return str;
}

void HttpRequest::parseHeaders(const std::string& rawHeaders)
{
    std::istringstream stream(rawHeaders);
    std::string line;
    int hostFlag = 0;

    if (!std::getline(stream, line) || line.empty()){
        throwHttpError(400, "Invalid HTTP request: empty request line");
    }

    std::istringstream requestLineStream(line);
    requestLineStream >> this->method >> this->uri >> this->version;
    if ((this->method.empty() || this->uri.empty() || this->version.empty())){
        throwHttpError(400, "Invalid HTTP request: info missed in request line");
    }
    else if (this->version != "HTTP/1.1") {
        throwHttpError(505, "HTTP Version Not Supported");
    }
    else if (this->method != "GET" && this->method != "POST" && this->method != "DELETE") {
        throwHttpError(501, "Not Implemented: Unsupported HTTP method"); //? kayna f dispatcher method
    }
    else if (this->uri.find_first_not_of("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~:/?#[]@!$&'()*+,;=%") != std::string::npos)
    {
        throwHttpError(400, "Invalid HTTP request: URI contains invalid characters");
    }
    else if (this->uri.size() > 2048)
    {
        throwHttpError(414, "URI Too Long");
    }
    
    // Parse headers
    while (std::getline(stream, line)) {
        std::string trimmed_line = line;
        // Remove trailing \r from the line read by getline
        if (!trimmed_line.empty() && trimmed_line[trimmed_line.size() - 1] == '\r') {
            trimmed_line.erase(trimmed_line.size() - 1);
        }

        if (trimmed_line.empty()) { // end of header line
            if (hostFlag != 1) {
                throwHttpError(400, "Bad request: Host header missing or deplicated");
            }
            break;
        }

        size_t colon = line.find(":");
        if (colon == 0) {
            throwHttpError(400, "Bad request: Empty header key");
        }
        if (colon == std::string::npos) {
            throwHttpError(400, "Bad request: invalid header line (missing colon)");
        }

        std::string key = line.substr(0, colon);
        std::string value = line.substr(colon + 1);

        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t\r") + 1); // remove trailing whitespace and \r
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t\r") + 1); // remove trailing whitespace and \r

        this->headers[key] = value;

        if (toLower(key) == "host") {
            if (value.empty()) {
                throwHttpError(400, "Bad request: Host header value empty");
            }
            hostFlag = 1;
        }
    }

        if (this->uri.find("http://", 0) == 0)
        {
            size_t host_start_pos = 7; // length of "http://"
            size_t path_start_pos = this->uri.find('/', host_start_pos);
            std::string host_in_uri;
        
            if (path_start_pos == std::string::npos) {
                // URI is like "http://www.example.com" with no path
                host_in_uri = this->uri.substr(host_start_pos);
                this->uri = "/"; // The path is just the root
            } else {
                // URI is like "http://www.example.com/path"
                host_in_uri = this->uri.substr(host_start_pos, path_start_pos - host_start_pos);
                this->uri = this->uri.substr(path_start_pos); // Update URI to just be the path
            }
        
            // l host d request khass tkon bhal host li 
            std::string host_header = GetHeader("host");
            if (host_header.find(':') != std::string::npos) {
                host_header = host_header.substr(0, host_header.find(':'));
            }
        
            if (host_in_uri != host_header) {
                throwHttpError(400, "Host in request URI does not match Host header");
            }
        }

    // After parsing all headers, determine body protocol if content-length or transfer-encoding
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
    // For methods other than POST, no body is expected
    this->isChunked = false;
    this->contentLength = 0;
    }
}

bool HttpRequest::parseBody(std::string& connectionBuffer, long maxBodySize) {
    if (!isChunked && contentLength == 0) {
        // No body expected already done;
        return true;
    }

    if (!isChunked) { // Content-Length body
        if (contentLength > maxBodySize)
            throwHttpError(413, "request entity Too Large");
        if (contentLength > 0 && connectionBuffer.length() >= static_cast<size_t>(contentLength)) {
            this->body.append(connectionBuffer.substr(0, contentLength));
            connectionBuffer.erase(0, contentLength);
            return true; // Body is complete
        }
        // Not enough data yet, will return false and wait for more
        return false;
    } else { // Chunked body
        // Call the incremental chunked decoder
        return decodeChunked(connectionBuffer, this->body);
    }
}

// --- decodeChunkedIncremental implementation ---
// This function will consume valid chunks from 'buffer' and append to 'decodedOutput'
// Returns true if the 0-chunk (end of message) is found and consumed.
bool HttpRequest::decodeChunked(std::string& buffer, std::string& decodedOutput) {
    size_t pos = 0;
    while (pos < buffer.length()) {
        size_t newline_pos = buffer.find("\r\n", pos);
        if (newline_pos == std::string::npos) {
            // Not enough data for chunk size line or trailing CRLF
            return false;
        }

        std::string size_hex_str = buffer.substr(pos, newline_pos - pos);
        std::istringstream ss(size_hex_str);
        long chunkSize;
        ss >> std::hex >> chunkSize;

        if (ss.fail() || !ss.eof()) { // Check for parsing errors or extra characters on size line
            throwHttpError(400, "Invalid chunk size format");
        }

        if (chunkSize == 0) { // End of chunked message
            // Check for the final CRLF after the 0-chunk
            if (buffer.length() < newline_pos + 4) { // Need "0\r\n\r\n"
                return false; // Not enough data for final CRLF
            }
            if (buffer.substr(newline_pos, 4) != "\r\n\r\n") {
                if (buffer.substr(newline_pos, 2) != "\r\n") { // Should be 0\r\n
                    throwHttpError(400, "Expected CRLF after 0-chunk size");
                }
            }
            buffer.erase(0, newline_pos + 4); // Consume "0\r\n\r\n"
            return true; // Body is complete
        }

        // Check if full chunk data + CRLF is available
        size_t expected_total_chunk_len = (newline_pos - pos) + 2 + chunkSize + 2; // size_hex + \r\n + data + \r\n
        if (buffer.length() < pos + expected_total_chunk_len) {
            // Not enough data for the full chunk (size, data, and trailing \r\n)
            return false;
        }

        // Extract chunk data
        size_t chunk_data_start = newline_pos + 2;
        decodedOutput.append(buffer.substr(chunk_data_start, chunkSize));

        // Check for chunk trailing CRLF
        size_t chunk_crlf_pos = chunk_data_start + chunkSize;
        if (buffer.substr(chunk_crlf_pos, 2) != "\r\n") {
            throwHttpError(400, "Expected CRLF after chunk data");
        }

        // Consume the processed chunk from the buffer
        buffer.erase(0, chunk_crlf_pos + 2); // Erase up to and including the chunk's trailing CRLF
        pos = 0; // Reset position as buffer has been modified
    }
    return false; // Not complete yet, or no more full chunks could be processed
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

