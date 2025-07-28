/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: baouragh <baouragh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 16:21:21 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/27 13:03:19 by baouragh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// HttpRequest.hpp
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
    long contentLength;
    bool isChunked;
    long bodyReceived;
    bool headersParsed;
    std::string UploadDirectory;
    std::string SessionId;

    HttpRequest();

    void parseHeaders(const std::string& rawHeaders);
    void parseRequestLine(const std::string& line);
    void parseHeaderLine(const std::string& line, int& hostFlag);
    void validateAbsoluteUri();
    void determineBodyProtocol();

    bool parseBody(std::string& connectionBuffer, long maxBodySize); // Will be called incrementally

    bool decodeChunked(std::string& buffer, std::string& decodedOutput, long maxBodySize);
    bool parseChunkSize(const std::string& sizeHex, long& chunkSize);
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
