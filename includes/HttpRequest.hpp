/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: youness <youness@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 16:21:21 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/10 18:47:51 by youness          ###   ########.fr       */
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

    HttpRequest();

    void parseHeaders(const std::string& rawHeaders);

    bool parseBody(std::string& connectionBuffer, long maxBodySize); // Will be called incrementally

    bool decodeChunked(std::string& buffer, std::string& decodedOutput);
    void throwHttpError(int statusCode, const std::string& message);
    std::string GetHeader(std::string target) const;
};

#endif
