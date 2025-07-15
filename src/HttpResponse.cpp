/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 17:21:11 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/13 21:03:17 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "../includes/HttpServer.hpp"

HttpResponse::HttpResponse()
    : version("HTTP/1.1"), statusCode(200), statusText("OK"), body("")
{
}

std::string HttpResponse::toString() const
{
    std::ostringstream response;
    response << version << " " << statusCode << " " << statusText << "\r\n";
    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it) {
        response << it->first << ": " << it->second << "\r\n";
    }
    response << "\r\n";
    response << body;
    return response.str();
}
