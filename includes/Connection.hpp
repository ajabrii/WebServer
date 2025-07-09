/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 16:21:00 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/09 13:23:40 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// Connection.hpp
#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <string>
#include <netinet/in.h>

class Connection {
private:
    int client_fd;
    sockaddr_in client_addr;
    std::string buffer;
    bool headersDone;
    // HttpRequest req;

public:
    Connection();
    Connection(int fd, const sockaddr_in& addr);
    ~Connection();

    int getFd() const;
    std::string readData(); // i remove const here to work with buffer in readData() function
    void writeData(const std::string& response) const;
    void closeConnection();
    std::string& getBuffer();
    void clearBuffer();
};

#endif
