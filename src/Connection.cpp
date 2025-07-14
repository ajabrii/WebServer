/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 17:21:04 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/14 12:00:11 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


# include "../includes/HttpServer.hpp"

Connection::Connection() : client_fd(-1), lastActivityTime(std::time(0)), keepAlive(false), requestCount(0)
{
    std::memset(&client_addr, 0, sizeof(client_addr));
}

Connection::Connection(int fd, const sockaddr_in& addr)
    : client_fd(fd), client_addr(addr), lastActivityTime(std::time(0)), keepAlive(false), requestCount(0)
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

std::string Connection::readData()
{
    char tmp[BUFFER_SIZE];
    ssize_t bytesRead = recv(client_fd, tmp, BUFFER_SIZE, 0);
    if (bytesRead < 0)
        throw std::runtime_error("Error: Failed to read from client");

    updateLastActivity();
    buffer.append(tmp, bytesRead);
    return std::string(buffer);
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
