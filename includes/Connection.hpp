/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 16:21:00 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/13 18:23:49 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <string>
#include <netinet/in.h>
# define BUFFER_SIZE 8192
class Connection
{
    private:
        int client_fd;
        sockaddr_in client_addr;
        std::string buffer;
    public:
        Connection();
        Connection(int fd, const sockaddr_in& addr);
        ~Connection();

        int getFd() const;
        std::string readData();
        void writeData(const std::string& response) const;
        void closeConnection();
        std::string& getBuffer();
        void clearBuffer();
};

#endif
