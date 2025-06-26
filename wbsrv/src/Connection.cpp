/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 17:21:04 by ajabri            #+#    #+#             */
/*   Updated: 2025/06/26 17:23:44 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


# include "../includes/HttpServer.hpp"

// Implementation
Connection::Connection() : client_fd(-1) {
    std::memset(&client_addr, 0, sizeof(client_addr));
}

Connection::Connection(int fd, const sockaddr_in& addr)
    : client_fd(fd), client_addr(addr) {}

Connection::~Connection() {
    closeConnection();
}

int Connection::getFd() const {
    return client_fd;
}

std::string Connection::readData() const {
    char buffer[4096];
    ssize_t bytesRead = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (bytesRead < 0)
        throw std::runtime_error("Failed to read from client");

    buffer[bytesRead] = '\0';
    return std::string(buffer);
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
