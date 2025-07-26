/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: baouragh <baouragh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 17:21:11 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/26 17:31:12 by baouragh         ###   ########.fr       */
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
    for (std::multimap<std::string, std::string>::const_iterator it = CookiesHeaders.begin(); it != CookiesHeaders.end(); ++it) {
        response << it->first << ": " << it->second << "\r\n";
    }
    response << "\r\n";
    response << body;
    return response.str();
}

void HttpResponse::SetCookieHeaders(HttpRequest &req)
{
    std::map<std::string, std::string> sessionData = req.sessionData;
    std::map<std::string, std::string> cookies = req.cookies;
    std::string sessionPath = req.sessionPath;
    

    for (std::map<std::string, std::string>::const_iterator it = cookies.begin(); it != cookies.end(); ++it)
    {
        sessionData[it->first] = it->second;
    }

    std::ofstream ofs(sessionPath.c_str(), std::ios::trunc);
    for (std::map<std::string, std::string>::const_iterator it = sessionData.begin(); it != sessionData.end(); ++it)
    {
        ofs << it->first << "=" << it->second << "\n";
    }
    ofs.close();
    // this->CookiesHeaders = ;
}

