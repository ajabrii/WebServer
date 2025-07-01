/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigInterpreter.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ytarhoua <ytarhoua@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 15:11:31 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/01 19:10:33 by ytarhoua         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "../includes/ConfigInterpreter.hpp"


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
    // std::cout << filePath << "\n";
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
    // for (size_t i = 0; i < ConfigData.size(); ++i)
    // {
    //     std::cout << "std::line :: " << ConfigData[i] << "\n";
    // }
    infile.close();
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
    //! handle comment in the same line ex: error_page 404 = /404.html; #comment
    //? done
    if (line.empty() || line.find('#') != std::string::npos || Isspaces(line))
        return true;
    return false;
}


void ConfigInterpreter::parse() 
{
    ServerConfig current_server;
    RouteConfig current_route;
    std::string line;

    // for (size_t i = 0; i < ConfigData.size(); i++)
    // {
    //     std::cout << "std::line :: " << ConfigData[i] << "\n";
    // }
    for (size_t i = 0; i < ConfigData.size(); i++)
    {
        line = ConfigData[i];
        if (IsComment(line))
            continue;

        if (line == "server")
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
        if (ext != ".py" && ext != ".php")
            throw std::runtime_error("unsupported extension ");
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

    if (key == "host") {
        if (!server.host.empty()) //? handle duplicate host
            throw std::runtime_error("Duplicate 'host' entry in server block.");
        server.host = value;
    }
    else if (key == "port")
    {
        // std::cout << "value :::::: " << value << "\n"; 
        if (server.port != 0) //? handle duplicate port
            throw std::runtime_error("Duplicate 'port' entry in server block.");
        if (value.size() > 4)
            throw std::runtime_error("invalid port ::" + value);
        server.port = std::atoi(value.c_str());
    }
    else if (key == "server_name")
    {
        std::stringstream ss(value);
        std::string name;
        while (ss >> name){
            for (size_t i = 0; i < server.serverName.size(); i++)
                if (server.serverName[i] == name)
                    throw std::runtime_error("name allready taken");
            server.serverName.push_back(name);
        }
    }
    else if (key == "client_max_body_size") //? i have to handle if number is negative
    {
        if (value.empty()) //? handle empty value
            throw std::runtime_error("Invalid client_max_body_size value: " + value);
        else if (atoi(value.c_str()) <= 0)
            throw std::runtime_error("negative client_max_body_size value: " + value);
        if (!value.empty() && value[value.size() - 1] == 'M') {
            value.erase(value.size() - 1); //* remove 'M'
            server.clientMaxBodySize = std::atoi(value.c_str()) * 1024 * 1024; //? if size in megabyts
        }
        else
            server.clientMaxBodySize = std::atoi(value.c_str()); //? if i set default in byts
        if (server.clientMaxBodySize > 10485760)
            throw std::runtime_error("invalid size. please enter less than 10M body size"); //? not sure
        // std::cout << "size is :: " << server.clientMaxBodySize << "\n";
    }
    else if (key.find("error_page") == 0)
    {
        std::stringstream ss(line);
        std::string temp;
        ss >> temp; // skip "error_page"
        std::vector<int> codes;
        std::string token;
        // Collect all codes (integers) until the last token (path)
        while (ss >> token)
        {
            // If token is all digits, treat as code
            bool isNumber = isNum(token);
            if (isNumber)
                codes.push_back(std::atoi(token.c_str()));
            else
            {
                // token is the path, assign to all codes
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
        if (serverConfigs[i].port == 0)
            throw std::runtime_error("Port is not set for a server block.");
        if (serverConfigs[i].clientMaxBodySize <= 0) //? handle max body size
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
    }
}
