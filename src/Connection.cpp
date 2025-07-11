/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: youness <youness@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 17:21:04 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/10 18:47:05 by youness          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


# include "../includes/HttpServer.hpp"

// Implementation
Connection::Connection() : client_fd(-1), buffer(""), requestState(READING_HEADERS),
                         contentLength(0), isChunked(false){
    std::memset(&client_addr, 0, sizeof(client_addr));
}

Connection::Connection(int fd, const sockaddr_in& addr)
    : client_fd(fd), client_addr(addr), buffer(""), requestState(READING_HEADERS),
                         contentLength(0), isChunked(false){}

Connection::~Connection() {
    closeConnection();
}

int Connection::getFd() const {
    return client_fd;
}

void Connection::readData(HttpServer* server) {
    char tmp[4096];
    ssize_t bytesRead = recv(client_fd, tmp, sizeof(tmp), 0);

    if (bytesRead < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // No data available right now. Not an error.
            return;
        } else {
            // A real error occurred with recv
            // Consider logging the actual errno value for debugging
            throw std::runtime_error("Failed to read from client socket: " + std::string(strerror(errno)));
        }
    } else if (bytesRead == 0) {
        // Client closed the connection
        throw std::runtime_error("Client disconnected (bytesRead == 0)"); // This should trigger connection cleanup
    }

    // Append the received data to the connection's buffer
    buffer.append(tmp, bytesRead);
    std::cout << "Debug: Read " << bytesRead << " bytes. Buffer size: " << buffer.length() << std::endl;

    if (requestState == READING_HEADERS) {
        size_t headerEndPos = buffer.find("\r\n\r\n");
        if (headerEndPos != std::string::npos) {
            // Headers are complete!
            std::string rawHeaders = buffer.substr(0, headerEndPos + 4); // Include the \r\n\r\n terminator
            buffer.erase(0, headerEndPos + 4); // Remove headers from the buffer, leaving only potential body data

                currentRequest.parseHeaders(rawHeaders);

                this->contentLength = currentRequest.contentLength;
                this->isChunked = currentRequest.isChunked;

                if (currentRequest.isChunked) {
                    requestState = READING_BODY_CHUNKED;
                } else if (currentRequest.contentLength > 0) {
                    requestState = READING_BODY_CONTENT_LENGTH;
                } else {
                    // No body expected
                    requestState = REQUEST_COMPLETE;
                }
                std::cout << "Debug: Headers parsed. State changed to: " << requestState << std::endl;

        }
    }
    
    // kmel la header salaw and request mazal masalat 
    if (requestState != READING_HEADERS && requestState != REQUEST_COMPLETE) {
            bool bodyComplete = currentRequest.parseBody(buffer, server->getConfig().clientMaxBodySize); // Pass buffer by reference
            if (bodyComplete) {
                requestState = REQUEST_COMPLETE;
                std::cout << "Debug: Body parsed. State changed to: " << requestState << std::endl;
            }
    }

    // If REQUEST_COMPLETE, the main loop can now process currentRequestL
    // and then call Connection::reset() for the next request on keep-alive
    // Connection::reset() should clear currentRequest and set requestState to READING_HEADERS
    if (requestState == REQUEST_COMPLETE) {
        std::cout << "Debug: Request fully received and parsed for fd " << client_fd << std::endl;
    }
}

void Connection::writeData(const std::string& response) const {
    ssize_t bytesSent = send(client_fd, response.c_str(), response.size(), 0);
    if (bytesSent < 0)
        throw std::runtime_error("Failed to write to client");
}

void Connection::closeConnection() {
    if (client_fd != -1) {
        close(client_fd);
        client_fd = -1;
    }
}

HttpRequest& Connection::getCurrentRequest() {
    return currentRequest;
}

bool Connection::isRequestComplete() const {
    return requestState == REQUEST_COMPLETE;
}

void Connection::reset() {
    buffer.clear();
    currentRequest = HttpRequest(); // Resetiha b constractor jdid;
    contentLength = 0;
    isChunked = false;
    requestState = READING_HEADERS;
    std::cout << "Debug: Connection " << client_fd << " reset for next request." << std::endl;
}