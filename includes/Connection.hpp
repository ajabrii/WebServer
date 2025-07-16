/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: baouragh <baouragh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: Invalid date        by                   #+#    #+#             */
/*   Updated: 2025/07/16 09:25:28 by baouragh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */



#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include "../includes/HttpRequest.hpp"
#include <string>
#include <netinet/in.h>
#include <ctime>
# define BUFFER_SIZE 8192
# define KEEP_ALIVE_TIMEOUT 60  // 60 seconds timeout for keep-alive connections

class HttpServer;


enum RequestState {
    READING_HEADERS,
    READING_BODY_CONTENT_LENGTH,
    READING_BODY_CHUNKED,
    REQUEST_COMPLETE,
    // ERROR_STATE // Added for explicit error handling
};
class Connection
{
    private:
        int client_fd;
        sockaddr_in client_addr;
        std::string buffer;
        time_t lastActivityTime;
        bool keepAlive;
        int requestCount;
        
        RequestState requestState;
        long contentLength; // Use long to match potential large sizes
        bool isChunked;
        HttpRequest currentRequest;
        HttpServer* server;
        
    public:
        Connection();
        Connection(int fd, const sockaddr_in& addr);
        ~Connection();

        int getFd() const;
        void readData(HttpServer* server);
        void writeData(const std::string& response) const;
        void closeConnection();
        std::string& getBuffer();
        void clearBuffer();// ! remove later on
        
        // Keep-alive related methods
        void updateLastActivity();
        bool isKeepAlive() const;
        void setKeepAlive(bool keepAlive);
        bool isTimedOut() const;
        int getRequestCount() const;
        void incrementRequestCount();
        void resetForNextRequest();

        HttpRequest& getCurrentRequest();
        bool isRequestComplete() const; // New helper to check state for main loop
        bool isConnectionClosed() const; // Check if connection is closed
        void reset();
};

#endif
