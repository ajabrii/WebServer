/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: baouragh <baouragh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 16:21:00 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/07 14:19:19 by baouragh         ###   ########.fr       */
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

public:
    Connection();
    Connection(int fd, const sockaddr_in& addr);
    ~Connection();

    int getFd() const;
    std::string readData(); // i remove const here to work with buffer in readData() function
    void writeData(const std::string& response) const;
    void closeConnection();
    // std::string getIP() const;
};

#endif
