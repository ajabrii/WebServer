/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigInterpreter.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: youness <youness@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 15:11:31 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/20 14:27:20 by youness          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


# include "../includes/ConfigInterpreter.hpp"
#include <algorithm> 
#include <climits>
#include <cstddef>


ConfigInterpreter::ConfigInterpreter() : serverFlag(false), routeFlag(false){}

void ConfigInterpreter::getConfigData(std::string filePath)
{
    std::string line;
    size_t commentPos;
    std::vector<std::string> blockKeywords;
    blockKeywords.push_back("server");
    blockKeywords.push_back("location");
    blockKeywords.push_back("route");
    bool matched;
    std::string keyword;
    // Check file extension (.config or .yaml)
    if (!hasValidExtension(filePath)) {
        throw std::runtime_error("Config file must have .config or .yaml extension");
    }
    std::ifstream infile(filePath.c_str());
    if (!infile.is_open())
        throw std::runtime_error("Cannot open config file");
    if (infile.peek() == std::ifstream::traits_type::eof())
        throw std::runtime_error("Config file is empty");

    while (std::getline(infile, line))
    {
        commentPos = line.find('#'); //?  here i handle the position of the comment like "server { #comment"
        if (commentPos != std::string::npos)
            line = line.substr(0, commentPos);

        line = trim(line);
        if (line.empty())
            continue;
        matched = false;
        for (size_t i = 0; i < blockKeywords.size(); ++i) {
            keyword = blockKeywords[i];

            if (line == keyword) {
                ConfigData.push_back(keyword);
                matched = true;
                break;
            }
            //? handell this problem :: "keyword {" or "keyword  \n {"
            if (line.find(keyword) != std::string::npos && line.find("{") != std::string::npos) {
                size_t keyPos = line.find(keyword);
                size_t bracePos = line.find("{");

                if (keyPos < bracePos) {
                    ConfigData.push_back(trim(line.substr(0, bracePos)));
                    ConfigData.push_back("{");
                    matched = true;
                    break;
                }
            }
        }

        if (!matched) {
            ConfigData.push_back(clean_line(line));
        }
    }
    infile.close();
}

bool ConfigInterpreter::hasValidExtension(const std::string& filePath) const
{
    size_t dotPos = filePath.rfind('.');
    if (dotPos == std::string::npos)
        return false;
    std::string ext = filePath.substr(dotPos);
    return (ext == ".config" || ext == ".yaml");
}

std::string ConfigInterpreter::trim(const std::string& line) {
    size_t start = line.find_first_not_of(" \t");
    size_t end = line.find_last_not_of(" \t");
    if (start == std::string::npos)
        return "";
    return line.substr(start, end - start + 1);
}

std::string ConfigInterpreter::clean_line(std::string line)
{
    // Trim leading and trailing whitespace
    line.erase(0, line.find_first_not_of(" \t\n\r"));
    line.erase(line.find_last_not_of(" \t\n\r") + 1);

    // Remove trailing semicolon if it exists
    if (!line.empty() && line[line.size() - 1] == ';')
    {
        line.erase(line.size() - 1);
    }
    return line;
}

std::string ConfigInterpreter::toLower(std::string str){
    std::string result = str;
    for (size_t i = 0; i < result.size(); ++i) {
        result[i] = std::tolower(static_cast<unsigned char>(result[i]));
    }
    return result;
}

bool ConfigInterpreter::Isspaces(const std::string& line)
{
    for (size_t i = 0; i < line.length(); i++)
    {
        if (!std::isspace(line[i]))
            return false;
    }
    return true;
}

bool ConfigInterpreter::IsComment(const std::string& line)
{
    if (line.empty() || line.find('#') != std::string::npos || Isspaces(line))
        return true;
    return false;
}


void ConfigInterpreter::parse() 
{
    ServerConfig current_server;
    RouteConfig current_route;
    std::string line;

    for (size_t i = 0; i < ConfigData.size(); i++)
    {
        line = ConfigData[i];
        if (IsComment(line))
            continue;

        if (toLower(line) == "server")
        {
            if (i + 1 < ConfigData.size() && ConfigData[i + 1] == "{")
            {
                serverFlag = 1;
                ++i;
                continue;
            }
            throw std::runtime_error("Invalid server block syntax");
        }

        if (line == "}")
        {
            if (routeFlag)
            {
                current_server.routes.push_back(current_route);
                current_route = RouteConfig();
                routeFlag = 0;
            }
            else if (serverFlag)
            {
                serverConfigs.push_back(current_server);
                current_server = ServerConfig();
                serverFlag = 0;
            } 
            else
            {
                throw std::runtime_error("Invalid config file syntax: unexpected closing brace '}' without a matching opening block");
            }
            continue;
        }

        if (line == "{")
        {
            if (!serverFlag && !routeFlag)
                throw std::runtime_error("Invalid config file syntax: unexpected '{' without a preceding block keyword");
        }

        if (line.compare(0, 5, "route") == 0 || line.compare(0, 8, "location") == 0)
        {
            if (i + 1 < ConfigData.size() && ConfigData[i + 1] == "{")
            {
                current_route.path = extractPathFromRouteLine(line);
                // std::cout << "route path::::::::" << current_route.path << "\n";
                routeFlag = 1;
                ++i;
                continue;
            }
            throw std::runtime_error("Invalid route block syntax");
        }

        if (serverFlag)
        {
            if (routeFlag)
            {
                parseRouteLine(current_route, line);
            }
            else
            {
                parseServerLine(current_server, line);
            }
        }
    }
    if ((routeFlag || serverFlag))
        throw std::runtime_error("Invalid config file syntax: expected '}' to end");
}

std::string ConfigInterpreter::extractPathFromRouteLine(const std::string& line) {
    size_t start = 0;
    std::string path;

    if ((line.find("route")) != std::string::npos) {
        start = line.find("route") + 5;
    } else if ((line.find("location")) != std::string::npos) {
        start = line.find("location") + 8;
    }

    size_t end = line.find("{"); //? i will change later (it stop at the end if line)
    if (end <= start)
        throw std::runtime_error("Invalid route line: " + line);

    path = line.substr(start, end - start);
    path = trim(path);
    if (path.find(' ') != std::string::npos)
        throw std::runtime_error("invalid path ::" + path);
    return path;
}

const std::vector<ServerConfig>& ConfigInterpreter::getServerConfigs() const {
    return serverConfigs;
}

void ConfigInterpreter::parseRouteLine(RouteConfig& route, const std::string& line)
{
    // std::cout << "--------------------------------------------->line: " << line << "\n";
    size_t equal = line.find('=');
    if (equal == std::string::npos)
        throw std::runtime_error("Invalid route line: " + line);

    std::string key = clean_line(line.substr(0, equal));
    std::string value = clean_line(line.substr(equal + 1));
    
    key = toLower(key); // bach n9bel kolchi 
    if (key == "methods")
    {
        std::stringstream ss(value);
        std::string method;
        while (ss >> method)
            if (method == "GET" || method == "POST" || method == "DELETE")
                route.allowedMethods.push_back(method);
            else 
                throw std::runtime_error("Unkownen method :: " + method);
    }
    else if (key == "indexfile")
    {
        route.indexFile = value;
    }
    else if (key == "redirect")
        route.redirect = value;
    else if (key == "root")
        route.root = value;
    else if (key == "directory_listing")
    {
        if (value == "on")
            route.directory_listing = true;
        else
            route.directory_listing = false;
    }
    else if (key == "upload_path") {
        if (!route.uploadDir.empty()) {
            throw std::runtime_error("Duplicate 'upload_path' entry in route block.");
        }
        route.uploadDir = value;
    }
    else if (key == "cgi")
    {
        std::stringstream line(value);

        std::stringstream countStream(value);
        int inputCount = 0;
        std::string tempInput;
        while (countStream >> tempInput) ++inputCount;
        if (inputCount != 2)
            throw std::runtime_error("invalid CGI input");
        std::string ext;
        std::string path;
        line >> ext;
        line >> path;
        // if (ext != ".py" && ext != ".php")
        //     throw std::runtime_error("unsupported extension ");
        route.cgi[ext] = path;
    }
    else
        throw std::runtime_error("Unknown route option: " + key);
}

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

void ConfigInterpreter::parseServerLine(ServerConfig& server, const std::string& line)
{
    size_t equal = line.find('=');
    if (equal == std::string::npos && line.find("error_page") != 0)
        throw std::runtime_error("Invalid server line: " + line);

    std::string key = trim(line.substr(0, equal));
    std::string value = trim(line.substr(equal + 1));

    key = toLower(key);
    if (key == "host") {
        if (!server.host.empty())
            throw std::runtime_error("Duplicate 'host' entry in server block.");
        server.host = value;
    }
    else if (key == "port")
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
            // check duplicates
            if (std::find(server.port.begin(), server.port.end(), portNum) != server.port.end())
                throw std::runtime_error("Duplicate port in server block: " + portStr);
            server.port.push_back(portNum);
        }
        if (server.port.empty())
            throw std::runtime_error("No valid ports specified in server block.");
    }
    else if (key == "server_name")
    {
        std::stringstream ss(value);
        std::string name;
        while (ss >> name){
            if (std::find(server.serverName.begin(), server.serverName.end(), name) != server.serverName.end())
                throw std::runtime_error("Server name already taken: " + name);
            server.serverName.push_back(name);
        }
    }
    else if (key == "client_max_body_size")
    {
        if (value.empty())
            throw std::runtime_error("client_max_body_size cannot be empty");
            
        unsigned long long size = 0;
        std::string numStr = value;
        unsigned long long multiplier = 1;
        
        // Check for suffix and extract number
        if (!value.empty()) {
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
        }
        
        // kolhom numbers
        if (numStr.empty() || !isNum(numStr)) {
            throw std::runtime_error("Invalid client_max_body_size number: " + numStr);
        }
        
        // Convert to number (using strtoul for better error handling)
        char* endptr;
        unsigned long baseSize = std::strtoul(numStr.c_str(), &endptr, 10);
        if (*endptr != '\0' || baseSize == 0) {
            throw std::runtime_error("Invalid client_max_body_size number: " + numStr);
        }
        
        // Check for overflow before multiplication
        if (baseSize > (ULLONG_MAX / multiplier)) {
            throw std::runtime_error("client_max_body_size too large: " + value);
        }
        
        size = baseSize * multiplier;

        server.clientMaxBodySize = size;
    }
    else if (key.find("error_page") == 0)
    {
        std::stringstream ss(line);
        std::string temp;
        ss >> temp; // skip "error_page"
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
    else
        throw std::runtime_error("Unknown server option: " + key);
}

void ConfigInterpreter::checkValues() const
{
    for (size_t i = 0; i < serverConfigs.size(); i++)
    {
        if (serverConfigs[i].host.empty())
            throw std::runtime_error("Host is not set for a server block.");
        if (serverConfigs[i].port.size() == 0)
            throw std::runtime_error("Port is not set for a server block.");
        if (serverConfigs[i].clientMaxBodySize <= 0)
            throw std::runtime_error("Client max body size is not set correct for a server block.");
        for (size_t j = 0; j < serverConfigs[i].routes.size(); j++)
        {
            if (serverConfigs[i].routes[j].path.empty())
                throw std::runtime_error("Route path is not set.");
            if (serverConfigs[i].routes[j].root.empty())
                throw std::runtime_error("root is not set for route " + serverConfigs[i].routes[j].path);
            if (serverConfigs[i].routes[j].allowedMethods.empty())
                throw std::runtime_error("No methods specified for route " + serverConfigs[i].routes[j].path);
        }
        for (size_t k = i + 1; k < serverConfigs.size(); k++)
        {
            if (serverConfigs[i].host == serverConfigs[k].host)
            {
            for (size_t pi = 0; pi < serverConfigs[i].port.size(); ++pi)
            {
                for (size_t pk = 0; pk < serverConfigs[k].port.size(); ++pk)
                {
                if (serverConfigs[i].port[pi] == serverConfigs[k].port[pk])
                {
                    throw std::runtime_error(
                    "Duplicate host and port found in two server blocks: "
                    // + serverConfigs[i].host + ":" + std::to_string(serverConfigs[i].port[pi])
                    );
                }
                }
            }
            }
        }
    }
}


std::string ConfigInterpreter::getPathForCGI(char **envp) const
{
    std::string str;

    if (!envp || !*envp)
        return "";
    for (size_t i = 0; envp[i] != NULL; i++)
    {
        str = envp[i];
        if (str.find("PATH=") != std::string::npos && !(str.find("_PATH") != std::string::npos))
        {
            return envp[i];
        }
    }
    return "";
}