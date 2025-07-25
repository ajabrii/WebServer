/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: baouragh <baouragh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/14 12:00:00 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/25 17:06:59 by baouragh         ###   ########.fr       */
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
    std::cerr << "-------------------------------------------------------> Connection header: " << connection << std::endl;
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

std::string toLower(const std::string& s) {
    std::string result = s;
    for (size_t i = 0; i < result.size(); ++i)
        result[i] = std::tolower(result[i]);
    return result;
}

// Helper: Trim leading spaces
void ltrim(std::string& s) {
    size_t i = 0;
    while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) ++i;
    s.erase(0, i);
}
HttpResponse parseCgiOutput(const std::string& raw) 
{
    HttpResponse response;

    size_t headerEnd = raw.find("\r\n\r\n");
    if (headerEnd == std::string::npos) 
    {
        response.statusCode = 500;
        response.statusText = "Internal Server Error";
        response.body = "CGI script did not return valid headers.";
        return response;
    }

    std::string headerPart = raw.substr(0, headerEnd);
    std::string bodyPart = raw.substr(headerEnd + 4);

    std::istringstream headerStream(headerPart);
    std::string line;
    bool statusParsed = false;

    // Read first line
    if (std::getline(headerStream, line)) 
    {
        if (!line.empty() && *line.rbegin() == '\r')
            line.erase(line.size() - 1);

        if (line.compare(0, 5, "HTTP/") == 0) 
        {
            std::istringstream statusStream(line);
            std::string httpVersion;
            statusStream >> httpVersion >> response.statusCode;
            std::getline(statusStream, response.statusText);
            ltrim(response.statusText);
            statusParsed = true;
        } 
        else 
        {
            // Reset headerStream to re-parse first line
            headerStream.clear(); // Reset error flags
            headerStream.str(headerPart); // Rewind to start
        }
    }

    // Parse headers
    while (std::getline(headerStream, line)) 
    {
        if (!line.empty() && *line.rbegin() == '\r')
            line.erase(line.size() - 1);

        size_t colon = line.find(':');
        if (colon != std::string::npos) 
        {
            std::string key = line.substr(0, colon);
            std::string value = line.substr(colon + 1);
            ltrim(value);

            // Lowercase for lookup only
            std::string lowerKey = toLower(key);
            if (lowerKey == "status") 
            {
                std::istringstream statusStream(value);
                statusStream >> response.statusCode;
                std::getline(statusStream, response.statusText);
                ltrim(response.statusText);
                statusParsed = true;
            } 
            else 
            {
                response.headers[key] = value;
            }
        }
    }

    if (!statusParsed) 
    {
        response.statusCode = 200;
        response.statusText = "OK";
    }

    response.body = bodyPart;

    if (response.headers.find("Content-Length") == response.headers.end()) 
    {
        std::ostringstream oss;
        oss << response.body.size();
        response.headers["Content-Length"] = oss.str();
    }

    if (response.headers.find("Content-Type") == response.headers.end()) 
    {
        response.headers["Content-Type"] = "text/html";
    }

    return response;
}



