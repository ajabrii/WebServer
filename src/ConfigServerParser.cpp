/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigServerParser.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: youness <youness@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/26 19:15:00 by youness          #+#    #+#             */
/*   Updated: 2025/07/26 19:15:00 by youness          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/ConfigInterpreter.hpp"

bool isNum(const std::string& token)
{
    if (token.empty())
        return false;
    for (size_t i = 0; i < token.size(); ++i)
    {
        if (!std::isdigit(token[i]))
            return false;
    }
    return true;
}

void ConfigInterpreter::parseHostOption(ServerConfig& server, const std::string& value)
{
    if (!server.host.empty())
        throw std::runtime_error("Duplicate 'host' entry in server block.");
    server.host = value;
}

void ConfigInterpreter::parsePortOption(ServerConfig& server, const std::string& value)
{
    std::stringstream ss(value);
    std::string portStr;
    while (ss >> portStr) {
        if (portStr.size() > 5)
            throw std::runtime_error("Invalid port length: " + portStr);
        if (!isNum(portStr))
            throw std::runtime_error("Port is not numeric: " + portStr);
        int portNum = std::atoi(portStr.c_str());
        if (portNum <= 0 || portNum > 65535)
            throw std::runtime_error("Invalid port number: " + portStr);

        if (std::find(server.port.begin(), server.port.end(), portNum) != server.port.end())
            throw std::runtime_error("Duplicate port in server block: " + portStr);
        server.port.push_back(portNum);
    }
    if (server.port.empty())
        throw std::runtime_error("No valid ports specified in server block.");
}

void ConfigInterpreter::parseServerNameOption(ServerConfig& server, const std::string& value)
{
    std::stringstream ss(value);
    std::string name;
    while (ss >> name){
        if (std::find(server.serverName.begin(), server.serverName.end(), name) != server.serverName.end())
            throw std::runtime_error("Server name already taken: " + name);
        server.serverName.push_back(name);
    }
}

void ConfigInterpreter::parseClientMaxBodySizeOption(ServerConfig& server, const std::string& value)
{
    if (server.clientMaxBodySizeSet) {
        throw std::runtime_error("Duplicate 'client_max_body_size' entry in server block.");
    }

    if (value.empty())
        throw std::runtime_error("client_max_body_size cannot be empty");

    std::string numStr = value;
    unsigned long long multiplier = 1;

        char lastChar = std::tolower(value[value.size() - 1]);
        if (lastChar == 'k' || lastChar == 'm' || lastChar == 'g') {
            if (value.size() == 1) {
                throw std::runtime_error("client_max_body_size: suffix without number");
            }
            numStr = value.substr(0, value.size() - 1);

            if (lastChar == 'k') {
                multiplier = 1024ULL;
            } else if (lastChar == 'm') {
                multiplier = 1024ULL * 1024ULL;
            } else if (lastChar == 'g') {
                multiplier = 1024ULL * 1024ULL * 1024ULL;
            }
        }

    if (numStr.empty() || !isNum(numStr)) {
        throw std::runtime_error("Invalid client_max_body_size number: " + numStr);
    }

    char* endptr;
    unsigned long baseSize = std::strtoul(numStr.c_str(), &endptr, 10);
    if (*endptr != '\0') {
        throw std::runtime_error("Invalid client_max_body_size number: " + numStr);
    }

    if (baseSize > (ULLONG_MAX / multiplier)) {
        throw std::runtime_error("client_max_body_size too large: " + value);
    }

    server.clientMaxBodySize = baseSize * multiplier;
    server.clientMaxBodySizeSet = true;
}

void ConfigInterpreter::parseErrorPageOption(ServerConfig& server, const std::string& line)
{
    std::stringstream ss(line);
    std::string temp;
    ss >> temp;
    std::vector<int> codes;
    std::string token;
    while (ss >> token)
    {
        if (isNum(token))
            codes.push_back(std::atoi(token.c_str()));
        else
        {
            for (size_t i = 0; i < codes.size(); ++i)
                server.error_pages[codes[i]] = token;
            break;
        }
    }
    if (codes.empty())
        throw std::runtime_error("No error codes specified for error_page directive.");
}

void ConfigInterpreter::parseServerOption(ServerConfig& server, const std::string& key, const std::string& value, const std::string& line)
{
    if (key == "host") {
        parseHostOption(server, value);
    }
    else if (key == "port")
    {
        parsePortOption(server, value);
    }
    else if (key == "server_name")
    {
        parseServerNameOption(server, value);
    }
    else if (key == "client_max_body_size")
    {
        parseClientMaxBodySizeOption(server, value);
    }
    else if (key.find("error_page") == 0)
    {
        parseErrorPageOption(server, line);
    }
    else
        throw std::runtime_error("Unknown server option: " + key);
}

void ConfigInterpreter::parseServerBlock(ServerConfig& server, const std::string& line)
{
    size_t equal = line.find_last_of('=');
    if (equal == std::string::npos && line.find("error_page") != 0)
        throw std::runtime_error("Invalid server line: " + line);

    std::string key = trim(line.substr(0, equal));
    std::string value = trim(line.substr(equal + 1));

    key = toLower(key);
    parseServerOption(server, key, value, line);
}
