/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ytarhoua <ytarhoua@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 16:21:21 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/03 17:52:01 by ytarhoua         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// HttpRequest.hpp
#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include <string>
#include <map>

class HttpRequest {
public:
    std::string method;
    std::string uri;
    std::string version;
    std::map<std::string, std::string> headers;
    std::string body;
    int status;

    HttpRequest();
    static HttpRequest parse(const std::string& raw);
};

#endif
