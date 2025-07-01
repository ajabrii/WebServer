/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 16:21:00 by ajabri            #+#    #+#             */
/*   Updated: 2025/06/26 16:21:02 by ajabri           ###   ########.fr       */
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

public:
    Connection();
    Connection(int fd, const sockaddr_in& addr);
    ~Connection();

    int getFd() const;
    std::string readData() const;
    void writeData(const std::string& response) const;
    void closeConnection();
};

#endif
