/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Errors.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/07 11:31:49 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/13 19:50:33 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Errors.hpp"
#include <fstream>
#include <sstream>
#include <map>
#include <iostream>


//todo add a static map that holds status codes for errors and appropriete string response or map it to defualt html string
std::string Error::loadErrorPage(int statusCode, const ServerConfig& config)
{
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
    return "<html><body><h1>" + std::to_string(statusCode) + " Error</h1></body></html>";
}


void Error::logs(const std::string& message)
{
        std::cerr << "\033[1;31m[Error]\033[0m " << message << std::endl;
}