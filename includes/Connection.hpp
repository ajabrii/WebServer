/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: baouragh <baouragh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: Invalid date        by                   #+#    #+#             */
/*   Updated: 2025/07/28 12:58:53 by baouragh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */



#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include "../includes/HttpRequest.hpp"
#include "../includes/SessionInfos.hpp"
#include <string>
#include <netinet/in.h>
#include <ctime>
#include "CgiState.hpp"
# define BUFFER_SIZE 8192
# define KEEP_ALIVE_TIMEOUT 60

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
        long contentLength;
        bool isChunked;
        HttpRequest currentRequest;
        CgiState* cgiState;
        SessionInfos sessionInfos;
        

        
        public:
        Connection();
        Connection(int fd, const sockaddr_in& addr);
        ~Connection();
        
        int getFd() const;
        void readData(HttpServer* server);
        void writeData(const std::string& response) const;
        std::string getClientIP() const;
        void closeConnection();

        void updateLastActivity();
        time_t getLastActivity() const{
            return lastActivityTime;
        }
        bool isKeepAlive() const;
        void setKeepAlive(bool keepAlive);
        bool isTimedOut() const;
        int getRequestCount() const;
        void incrementRequestCount();
        void resetForNextRequest();

        HttpRequest& getCurrentRequest();
        bool isRequestComplete() const;
        bool isConnectionClosed() const;
        void reset();
        CgiState* getCgiState() const;
        void setCgiState(CgiState* cgiState);
        void setSessionInfos(const SessionInfos& sessionInfos);
        SessionInfos& getSessionInfos();
        std::string ipToString(uint32_t ip_net_order) const;
};
#endif
