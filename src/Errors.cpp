/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Errors.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/07 11:31:49 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/22 12:46:15 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Errors.hpp"
#include <fstream>
#include <sstream>
#include <map>
#include <iostream>

std::map<int, std::string> Error::defaultErrResponses;

static void initDefaultErrorResponses() {
    static bool initialized = false;
    if (!initialized) {
        Error::defaultErrResponses[400] = "<html><head><style>body{background:#f9f9f9;font-family:sans-serif;color:#333;text-align:center;padding:50px;}h1{font-size:24px;}}</style></head><body><h1>400 Bad Request</h1></body></html>";
        Error::defaultErrResponses[403] = "<html><head><style>body{background:#f9f9f9;font-family:sans-serif;color:#333;text-align:center;padding:50px;}h1{font-size:24px;}}</style></head><body><h1>403 Forbidden</h1></body></html>";
        Error::defaultErrResponses[404] = "<html><head><style>body{background:#f9f9f9;font-family:sans-serif;color:#333;text-align:center;padding:50px;}h1{font-size:24px;}}</style></head><body><h1>404 Not Found</h1></body></html>";
        Error::defaultErrResponses[405] = "<html><head><style>body{background:#f9f9f9;font-family:sans-serif;color:#333;text-align:center;padding:50px;}h1{font-size:24px;}}</style></head><body><h1>405 Method Not Allowed</h1></body></html>";
        Error::defaultErrResponses[413] = "<html><head><style>body{background:#f9f9f9;font-family:sans-serif;color:#333;text-align:center;padding:50px;}h1{font-size:24px;}}</style></head><body><h1>413 Payload Too Large</h1></body></html>";
        Error::defaultErrResponses[431] = "<html><head><style>body{background:#f9f9f9;font-family:sans-serif;color:#333;text-align:center;padding:50px;}h1{font-size:24px;}}</style></head><body><h1>431 Request Header Fields Too Large</h1></body></html>";
        Error::defaultErrResponses[500] = "<html><head><style>body{background:#f9f9f9;font-family:sans-serif;color:#333;text-align:center;padding:50px;}h1{font-size:24px;}}</style></head><body><h1>500 Internal Server Error</h1></body></html>";
        Error::defaultErrResponses[501] = "<html><head><style>body{background:#f9f9f9;font-family:sans-serif;color:#333;text-align:center;padding:50px;}h1{font-size:24px;}}</style></head><body><h1>501 Not Implemented</h1></body></html>";
        Error::defaultErrResponses[505] = "<html><head><style>body{background:#f9f9f9;font-family:sans-serif;color:#333;text-align:center;padding:50px;}h1{font-size:24px;}}</style></head><body><h1>505 HTTP Version Not Supported</h1></body></html>";
        initialized = true;
    }
}



std::string Error::loadErrorPage(int statusCode, const ServerConfig& config)
{
    initDefaultErrorResponses();
    std::map<int, std::string>::const_iterator it = config.error_pages.find(statusCode);
    if (it != config.error_pages.end())
    {
        const std::string& filePath = it->second;
        std::ifstream file(filePath.c_str());
        if (file)
        {
            std::stringstream buffer;
            buffer << file.rdbuf();
            return buffer.str();
        }
    }
    return defaultErrResponses[statusCode];
}


void Error::logs(const std::string& message)
{
        std::cerr << "\033[1;31m[Error]\033[0m " << message << std::endl;
}