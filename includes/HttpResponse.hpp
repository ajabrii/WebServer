/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 16:21:26 by ajabri            #+#    #+#             */
/*   Updated: 2025/06/26 16:21:37 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// HttpResponse.hpp
#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP

#include <string>
#include <map>

class HttpResponse {
public:
    std::string version;
    int statusCode;
    std::string statusText;
    std::map<std::string, std::string> headers;
    std::string body;

    HttpResponse();
    std::string toString() const;
};

#endif
