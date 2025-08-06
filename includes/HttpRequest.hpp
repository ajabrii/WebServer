/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 16:21:21 by ajabri            #+#    #+#             */
/*   Updated: 2025/08/06 11:15:35 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include <string>
#include <map>
#include <algorithm>

class HttpRequest {
public:
    std::string method;
    std::string uri;
    std::string version;
    std::map<std::string, std::string> headers;
    std::string body;
    unsigned long long contentLength;
    bool isChunked;
    long bodyReceived;
    bool headersParsed;
    std::string UploadDirectory;
    std::string SessionId;
    bool checkVrsHTTP0;

    HttpRequest();

    void parseHeaders(const std::string& rawHeaders);
    void parseRequestLine(const std::string& line);
    void parseHeaderLine(const std::string& line, int& hostFlag);
    void validateAbsoluteUri();
    void determineBodyProtocol();

    bool parseBody(std::string& connectionBuffer, unsigned long long maxBodySize); // Will be called incrementally

    bool decodeChunked(std::string& buffer, std::string& decodedOutput, unsigned long long maxBodySize);
    bool parseChunkSize(const std::string& sizeHex, unsigned long long& chunkSize);
    bool skipTrailerHeaders(std::string& buffer, size_t startPos);
    void throwHttpError(int statusCode, const std::string& message);
    std::string GetHeader(std::string target) const;

    class HttpException : public std::exception {
        private:
        int         statusCode;
        std::string message;

        public:
            HttpException(int code, const std::string& msg);
            virtual ~HttpException() throw();
            virtual const char* what() const throw();
            int getStatusCode() const;
    };
};

#endif
