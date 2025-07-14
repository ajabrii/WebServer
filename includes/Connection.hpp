/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 16:21:00 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/14 16:33:43 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <string>
#include <netinet/in.h>
#include <ctime>
# define BUFFER_SIZE 8192
# define KEEP_ALIVE_TIMEOUT 60  // 60 seconds timeout for keep-alive connections

class Connection
{
    private:
        int client_fd;
        sockaddr_in client_addr;
        std::string buffer;
        time_t lastActivityTime;
        bool keepAlive;
        int requestCount;
        
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
        
        // Keep-alive related methods
        void updateLastActivity();
        bool isKeepAlive() const;
        void setKeepAlive(bool keepAlive);
        bool isTimedOut() const;
        int getRequestCount() const;
        void incrementRequestCount();
        void resetForNextRequest();
};

#endif
