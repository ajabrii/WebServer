/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: baouragh <baouragh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 16:21:21 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/05 14:29:08 by baouragh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// HttpRequest.hpp
#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include <string>
#include <map>

class HttpRequest {
public:
    std::string method; // 
    std::string uri; // ?query ---> 
    // query --> ? --> script-path --> Path-info /path --> /
    std::string version; // http
    std::map<std::string, std::string> headers; // 
    std::string body; // POST 

    HttpRequest();
    static HttpRequest parse(const std::string& raw);
    std::string GetHeader(std::string target) const;
};

#endif
