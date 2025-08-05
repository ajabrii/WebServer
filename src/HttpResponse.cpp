/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: baouragh <baouragh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 17:21:11 by ajabri            #+#    #+#             */
/*   Updated: 2025/08/05 11:27:51 by baouragh         ###   ########.fr       */
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


void HttpResponse::SetCookieHeaders(Connection &conn)
{
    std::map<std::string, std::string> cookies = conn.getSessionInfos().getCookies();
    std::map<std::string, std::string> sessionData;
    const std::string sessionPath = "./session/" + conn.getSessionInfos().getSessionId();

    std::ifstream ifs(sessionPath.c_str());
    if (ifs)
    {
        std::string line;
        while (std::getline(ifs, line))
        {
            size_t sep = line.find('=');
            if (sep != std::string::npos)
            {
                std::string key = line.substr(0, sep);
                std::string val = line.substr(sep + 1);
                sessionData[key] = val;
            }
        }
        ifs.close();
    }
    for (std::map<std::string, std::string>::iterator it = sessionData.begin(); it != sessionData.end(); ++it)
        cookies[it->first] = it->second;
    for (std::map<std::string, std::string>::const_iterator it = cookies.begin(); it != cookies.end(); ++it)
        this->CookiesHeaders.insert(
            std::make_pair("Set-Cookie", it->first + "=" + it->second + "; Path=/; HttpOnly"));
    // std::ofstream ofs(sessionPath.c_str(), std::ios::trunc);
    // if (ofs)
    // {
    //     for (std::map<std::string, std::string>::const_iterator it = cookies.begin(); it != cookies.end(); ++it)
    //     {
    //         ofs << it->first << "=" << it->second << "\n";
    //     }
    //     ofs.close();
    // }
    // else
    //     std::cerr << "Error: Could not open session file for writing: " << sessionPath << std::endl;
}
