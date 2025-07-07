/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: baouragh <baouragh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 17:21:04 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/07 15:02:32 by baouragh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


# include "../includes/HttpServer.hpp"
# include "errno.h"

// Implementation
Connection::Connection() : client_fd(-1), headersDone(false){
    std::memset(&client_addr, 0, sizeof(client_addr));
}

Connection::Connection(int fd, const sockaddr_in& addr)
    : client_fd(fd), client_addr(addr),  headersDone(false){}

Connection::~Connection() {
    closeConnection();
}

int Connection::getFd() const {
    return client_fd;
}
// std::string Connection::getIP() const
// {
//     sockaddr *add;

//     add = (struct sockaddr*)&(this->client_addr);
//     return (add->sa_data);
// }
// void handleClient(ClientConnection &client) {
//     char temp[1024];
//     ssize_t n = recv(fd, temp, sizeof(temp), 0);
//     if (n <= 0) {
//         // error or closed connection
//         return;
//     }

//     buffer.append(temp, n);

//     if (!headersDone) {
//         size_t pos = buffer.find("\r\n\r\n");
//         if (pos != std::string::npos) {
//             // Headers done!
//             std::string headerPart = buffer.substr(0, pos + 4);
//             request = HttpRequest::parse(headerPart);

//             // Get Content-Length
//             std::string cl = request.headers["Content-Length"];
//             if (!cl.empty()) {
//                 contentLength = std::stoi(cl);
//             } else {
//                 contentLength = 0;
//             }

//             headersDone = true;

//             // Remove headers from buffer
//             buffer.erase(0, pos + 4);
//             bodyReceived = buffer.size();
//         }
//     }

//     if (headersDone && contentLength > 0) {
//         bodyReceived = buffer.size();
//         if (bodyReceived >= contentLength) {
//             // Body fully received!
//             request.body = buffer.substr(0, contentLength);
//             buffer.erase(0, contentLength);

//             // FULL REQUEST PARSED
//             std::cout << "Method: " << request.method << "\n";
//             std::cout << "URI: " << request.uri << "\n";
//             std::cout << "Body: " << request.body << "\n";

//             // Mark complete — handle response...
//         }
//     } else if (headersDone && contentLength == 0) {
//         // No body expected
//         //  FULL REQUEST PARSED
//     }
// }


std::string Connection::readData()
{
    char tmp[2048]; //? pay attantion to this i will change it later
    ssize_t bytesRead = recv(client_fd, tmp, sizeof(tmp) - 1, 0);
    if (bytesRead < 0)
        throw std::runtime_error("Failed to read from client");

    buffer.append(tmp, bytesRead);

    if (!headersDone) {
        size_t pos = buffer.find("\r\n\r\n");
        if (pos != std::string::npos) {
            headersDone = true;
        }
    }
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