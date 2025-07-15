/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/14 12:00:00 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/15 12:05:34 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Utils.hpp"



/*
 **** helper functions ***
*/

std::string clean_line(std::string line)
{
    line.erase(0, line.find_first_not_of(" \t\n\r"));
    line.erase(line.find_last_not_of(" \t\n\r") + 1);

    if (!line.empty() && line[line.size() - 1] == ';')
    {
        line.erase(line.size() - 1);
    }
    return line;
}

bool shouldKeepAlive(const HttpRequest& request)
{
    std::string connection = request.GetHeader("connection");
    std::transform(connection.begin(), connection.end(), connection.begin(), ::tolower); // http is case-insensitive
    
    if (connection == "close") 
        return false;
    return true;
}

void setConnectionHeaders(HttpResponse& response, bool keepAlive)
{
    if (keepAlive) {
        response.headers["Connection"] = "keep-alive";
        response.headers["Keep-Alive"] = "timeout=60, max=100";
    } else {
        response.headers["Connection"] = "close";
    }
}


