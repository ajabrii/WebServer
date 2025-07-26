/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: baouragh <baouragh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 16:21:26 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/26 19:06:05 by baouragh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// HttpResponse.hpp
#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP

#include <string>
#include <map>

typedef class HttpRequest HttpRequest;

class HttpResponse {
public:
    std::string version;
    int statusCode;
    std::string statusText;
    std::map<std::string, std::string> headers;
    std::multimap<std::string, std::string> CookiesHeaders;
    std::string body;
    std::string SessionPath;

    HttpResponse();
    std::string toString() const;
    void SetCookieHeaders(HttpRequest &req);
};

#endif
