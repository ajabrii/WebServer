/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigInterpreter.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 15:11:31 by ajabri            #+#    #+#             */
/*   Updated: 2025/08/02 09:52:48 by ajabri           ###   ########.fr       */
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
    if (filePath.empty()) {
        throw std::runtime_error("Config file path is empty");
    }
    if (!hasValidExtension(filePath)) {
        throw std::runtime_error("Config file must have .conf or .yaml extension");
    }
    std::ifstream infile(filePath.c_str());
    if (!infile.is_open())
        throw std::runtime_error("Cannot open config file");
    if (infile.peek() == std::ifstream::traits_type::eof())
        throw std::runtime_error("Config file is empty");

    while (std::getline(infile, line))
    {
        commentPos = line.find('#');
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
    return (ext == ".conf" || ext == ".yaml");
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
    line.erase(0, line.find_first_not_of(" \t\n\r"));
    line.erase(line.find_last_not_of(" \t\n\r") + 1);

    if (!line.empty() && line[line.size() - 1] == ';')
        line.erase(line.size() - 1);
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
                throw std::runtime_error("Invalid config file syntax: unexpected closing brace '}' without a matching opening block");

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
                routeFlag = 1;
                ++i;
                continue;
            }
            throw std::runtime_error("Invalid route block syntax");
        }

        if (serverFlag)
        {
            if (routeFlag)
                parseRouteBlock(current_route, line);
            else
                parseServerBlock(current_server, line);
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

    size_t end = line.find("{");
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

void ConfigInterpreter::parseMethodsOption(RouteConfig& route, const std::string& value)
{
    std::stringstream ss(value);
    std::string method;
    while (ss >> method)
        if (method == "GET" || method == "POST" || method == "DELETE")
            route.allowedMethods.push_back(method);
        else
            throw std::runtime_error("Unkownen method :: " + method);
}

void ConfigInterpreter::parseDirectoryListingOption(RouteConfig& route, const std::string& value)
{
    if (value == "on")
        route.directory_listing = true;
    else
        route.directory_listing = false;
}

void ConfigInterpreter::parseUploadPathOption(RouteConfig& route, const std::string& value)
{
    if (!route.uploadDir.empty()) {
        throw std::runtime_error("Duplicate 'upload_path' entry in route block.");
    }
    route.uploadDir = value;
}

void ConfigInterpreter::parseCgiOption(RouteConfig& route, const std::string& value)
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
    route.cgi[ext] = path;
}

void ConfigInterpreter::parseRouteOption(RouteConfig& route, const std::string& key, const std::string& value)
{
    if (key == "methods")
    {
        if (!route.allowedMethods.empty()) {
            throw std::runtime_error("Duplicate 'methods' entry in route block.");
        }
        parseMethodsOption(route, value);
    }
    else if (key == "root")
    {
        if (!route.root.empty()) {
            throw std::runtime_error("Duplicate 'root' entry in route block.");
        }
        route.root = value;
    }
    else if (key == "indexfile")
    {
        route.indexFile = value;
    }
    else if (key == "redirect")
        route.redirect = value;
    else if (key == "autoindex")
    {
        parseDirectoryListingOption(route, value);
    }
    else if (key == "upload_path") {
        parseUploadPathOption(route, value);
    }
    else if (key == "cgi")
    {
        parseCgiOption(route, value);
    }
    else
        throw std::runtime_error("Unknown route option: " + key);
}

void ConfigInterpreter::parseRouteBlock(RouteConfig& route, const std::string& line)
{
    size_t equal = line.find('=');
    if (equal == std::string::npos)
        throw std::runtime_error("Invalid route line: " + line);

    std::string key = clean_line(line.substr(0, equal));
    std::string value = clean_line(line.substr(equal + 1));
    key = toLower(key);
    parseRouteOption(route, key, value);
}

void ConfigInterpreter::checkValues()
{
    for (size_t i = 0; i < serverConfigs.size(); i++)
    {
        if (serverConfigs[i].host.empty())
            serverConfigs[i].host = "0.0.0.0";
        if (serverConfigs[i].port.size() == 0)
            throw std::runtime_error("Port is not set for a server block.");
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
                    "Duplicate host and port found in two server blocks: ");
                }
                }
            }
            }
        }

        std::set<std::string> seen_routes;
        for (size_t j = 0; j < serverConfigs[i].routes.size(); j++)
        {
            const std::string& path = serverConfigs[i].routes[j].path;
            if (!seen_routes.insert(path).second) {
                throw std::runtime_error("Duplicate route path found in server block: " + path);
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