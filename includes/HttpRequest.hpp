/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ytarhoua <ytarhoua@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 16:21:21 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/08 18:12:20 by ytarhoua         ###   ########.fr       */
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

    void parseBody(const std::string& rawBodyChunk); // Will be called incrementally

    std::string decodeChunked(const std::string& chunkedBody);
    void throwHttpError(int statusCode, const std::string& message);
    std::string GetHeader(std::string target) const;
};

#endif
