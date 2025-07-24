/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: baouragh <baouragh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 17:21:04 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/24 18:52:39 by baouragh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */



# include "../includes/HttpServer.hpp"
# include "../includes/Errors.hpp"
# define CGI_TIMEOUT_MINUTES 10

Connection::Connection() 
    : client_fd(-1), 
    lastActivityTime(std::time(0)),
    keepAlive(false),
    requestCount(0),
    requestState(READING_HEADERS),
    contentLength(0),
    isChunked(false),
    cgiState(NULL)
{
    std::memset(&client_addr, 0, sizeof(client_addr));
}

Connection::Connection(int fd, const sockaddr_in& addr)
    : client_fd(fd),
    client_addr(addr),
    lastActivityTime(std::time(0)),
    keepAlive(false),
    requestCount(0),
    requestState(READING_HEADERS),
    contentLength(0),
    isChunked(false),
    cgiState(NULL)
{
}

Connection::~Connection()
{
    closeConnection();
}

int Connection::getFd() const
{
    return client_fd;
}

std::string& Connection::getBuffer()
{
    return buffer;
}

void Connection::clearBuffer()
{
    buffer.clear();
}
/*
=== readData() reads the HTTP request from the client socket and stores it in a buffer ===

* recv() : a system call that works like read(), but is used for sockets.
@ Reads up to sizeof(tmp)-1 bytes from client_fd into tmp buffer.
? Note: the last param in recv is for changing the behavoir of recev in our case 0 mean just default job of read.

@ Appends the read data to 'buffer'.
@ Checks if HTTP headers have ended by searching for "\r\n\r\n".
*/

void Connection::readData(HttpServer* server)
{
    char tmp[4096];
    ssize_t bytesRead = recv(client_fd, tmp, sizeof(tmp), 0);

    if (bytesRead < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) { // fcheck wakha machi forbidden hit khdamin b poll
            // No data available right now. Not an error.
            return;
        } else {
            // A real error occurred with recv
            // Consider logging the actual errno value for debugging
            throw std::runtime_error("Failed to read from client socket: " + std::string(strerror(errno)));
        }
    } 
    else if (bytesRead == 0) {
        // Client closed the connection gracefully
        // For keep-alive connections, this is normal when client closes
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

void Connection::updateLastActivity() {
    lastActivityTime = std::time(0);
}

bool Connection::isKeepAlive() const {
    return keepAlive;
}

void Connection::setKeepAlive(bool ka) {
    keepAlive = ka;
}

bool Connection::isTimedOut() const {
    
    if (cgiState && cgiState->pid != -1) {
        return (std::time(0) - lastActivityTime) > CGI_TIMEOUT_MINUTES;
    }
    return (std::time(0) - lastActivityTime) > KEEP_ALIVE_TIMEOUT;
}

int Connection::getRequestCount() const {
    return requestCount;
}

void Connection::incrementRequestCount() {
    requestCount++;
}

void Connection::resetForNextRequest() {
    buffer.clear();
    updateLastActivity();
    this->cgiState = NULL; // Reset CGI state for the next request
}
/*
=== this function writes data aka response to the client ==
? send it's like write function but used for socket the last param is  like the recv is flag that twiks the behavior of send
*/
void Connection::writeData(const std::string& response) const
{
    ssize_t bytesSent = send(client_fd, response.c_str(), response.size(), 0);
    if (bytesSent < 0)
        throw std::runtime_error("Failed to write to client");
}

void Connection::closeConnection()
{
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

bool Connection::isConnectionClosed() const {
    return client_fd == -1;
}

CgiState* Connection::getCgiState() const
{
    return cgiState;
};
void Connection::setCgiState(CgiState* cgiState)
{
    this->cgiState = cgiState;
    if (cgiState)
    {
        cgiState->startTime = std::time(0);
        cgiState->headersParsed = false;
        cgiState->rawOutput.clear();
    }
}
std::string ipToString(uint32_t ip_net_order)
{
    unsigned char bytes[4];
    bytes[0] = (ip_net_order >> 24) & 0xFF;
    bytes[1] = (ip_net_order >> 16) & 0xFF;
    bytes[2] = (ip_net_order >> 8) & 0xFF;
    bytes[3] = ip_net_order & 0xFF;

    std::ostringstream oss;
    oss << (int)bytes[0] << "." << (int)bytes[1] << "." << (int)bytes[2] << "." << (int)bytes[3];
    return oss.str();
}


std::string Connection::getClientIP() const
{
    return ipToString(client_addr.sin_addr.s_addr);
}