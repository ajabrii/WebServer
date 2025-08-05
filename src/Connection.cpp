/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 17:21:04 by ajabri            #+#    #+#             */
/*   Updated: 2025/08/02 12:17:13 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */



# include "../includes/HttpServer.hpp"
# include "../includes/Errors.hpp"
# include "../includes/Utils.hpp"
# define CGI_TIMEOUT_MINUTES  (5 * 60)

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

void Connection::readData(HttpServer* server)
{
    char tmp[4096];

    ssize_t bytesRead = recv(client_fd, tmp, sizeof(tmp), 0);
    if (bytesRead < 0) {
            return;
    }
    buffer.append(tmp, bytesRead);
    if (requestState == READING_HEADERS) {
        size_t headerEndPos = buffer.find("\r\n\r\n");
        if (headerEndPos != std::string::npos) {
            std::string rawHeaders = buffer.substr(0, headerEndPos + 4);
            buffer.erase(0, headerEndPos + 4);
            currentRequest.parseHeaders(rawHeaders);
            this->contentLength = currentRequest.contentLength;
            this->isChunked = currentRequest.isChunked;
            if (currentRequest.isChunked) {
                requestState = READING_BODY_CHUNKED;
            } else if (currentRequest.contentLength > 0) {
                requestState = READING_BODY_CONTENT_LENGTH;
            } else {
                requestState = REQUEST_COMPLETE;
            }
        }
    }
    if (requestState != READING_HEADERS && requestState != REQUEST_COMPLETE) {
            bool bodyComplete = currentRequest.parseBody(buffer, server->getConfig().clientMaxBodySize);
            if (bodyComplete) {
                requestState = REQUEST_COMPLETE;
            }
    }
    if (requestState == REQUEST_COMPLETE) {
        ;
    }
}

void Connection::updateLastActivity()
{
    lastActivityTime = std::time(0);
}

bool Connection::isKeepAlive() const
{
    return keepAlive;
}

void Connection::setKeepAlive(bool ka)
{
    keepAlive = ka;
}

bool Connection::isTimedOut() const
{
    HttpResponse response;
    if (cgiState && cgiState->pid != -1)
    {
        bool isTimedOut = (std::time(0) - cgiState->startTime) > CGI_TIMEOUT_MINUTES;
        if(isTimedOut)
        {
            kill(cgiState->pid, SIGKILL);
            cgiState->pid = -1;
            response.statusCode = 504;
            response.statusText = "CGI script took too long to respond";
            response.body = "CGI script took too long to respond";
            response.headers["Content-Type"] = "text/html";
            response.headers["Content-Length"] = Utils::toString(response.body.size());
            this->writeData(response.toString());
        }
        return (isTimedOut);
    }
    return (std::time(0) - lastActivityTime) > KEEP_ALIVE_TIMEOUT;
}

int Connection::getRequestCount() const
{
    return requestCount;
}

void Connection::incrementRequestCount()
{
    requestCount++;
}

void Connection::resetForNextRequest()
{
    buffer.clear();
    updateLastActivity();
    if (cgiState) {
        delete cgiState;
        cgiState = NULL;
    }
    currentRequest = HttpRequest();
    contentLength = 0;
    isChunked = false;
    requestState = READING_HEADERS;
}

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

HttpRequest& Connection::getCurrentRequest()
{
    return currentRequest;
}

bool Connection::isRequestComplete() const
{
    return requestState == REQUEST_COMPLETE;
}

void Connection::reset()
{
    buffer.clear();
    currentRequest = HttpRequest();
    contentLength = 0;
    isChunked = false;
    requestState = READING_HEADERS;
    if (cgiState) {
        delete cgiState;
        cgiState = NULL;
    }
}

bool Connection::isConnectionClosed() const
{
    return client_fd == -1;
}

CgiState* Connection::getCgiState() const
{
    return cgiState;
}

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

std::string Connection::ipToString(uint32_t ip_net_order) const
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

SessionInfos& Connection::getSessionInfos()
{
    return sessionInfos;
}

void Connection::setSessionInfos(const SessionInfos& sessionInfos)
{
    this->sessionInfos = sessionInfos;
}


time_t Connection::getLastActivity() const
{
    return lastActivityTime;
}