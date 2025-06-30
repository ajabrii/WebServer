/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigInterpreter.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 15:11:31 by ajabri            #+#    #+#             */
/*   Updated: 2025/06/30 10:22:21 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "../includes/ConfigInterpreter.hpp"



ConfigInterpreter::ConfigInterpreter() {}

void ConfigInterpreter::parse(const std::string& filepath) 
{
    std::vector<std::string> lines = readFile(filepath);
    for (size_t i = 0; i < lines.size(); ++i) {
        std::string line = trim(lines[i]);

        if (line.empty() || line[0] == '#')
            continue;

        if (line == "server") {
            ++i;
            if (i >= lines.size() || trim(lines[i]) != "{")
                throw std::runtime_error("Expected '{' after 'server'");

            parseServerBlock(lines, i);
        }
    }
}

const std::vector<ServerConfig>& ConfigInterpreter::getServerConfigs() const {
    return serverConfigs;
}

void ConfigInterpreter::parseServerBlock(const std::vector<std::string>& lines, size_t& i)
{
    ServerConfig server;
    ++i; // move past '{'

    for (; i < lines.size(); ++i) {
        std::string line = trim(lines[i]);

        if (line.empty() || line[0] == '#')
            continue;

        if (line == "}") {
            serverConfigs.push_back(server);
            return;
        }

        if (line.substr(0, 5) == "route") {
            std::istringstream iss(line);
            std::string word, routePath;
            iss >> word >> routePath; // route /path

            ++i;
            if (i >= lines.size() || trim(lines[i]) != "{")
                // std::cout << i << std::endl;
                throw std::runtime_error("Expected '{' after 'route' error line : ");

            parseRouteBlock(server, lines, i);
            continue;
        }

        // Server-level directive parsing
        std::istringstream iss(line);
        std::string key, value;
        if (line.find('=') != std::string::npos) {
            getline(iss, key, '=');
            getline(iss, value, ';');
            key = trim(key);
            value = trim(value);

            if (key == "host") server.host = value;
            else if (key == "port") 
                server.port = atoi(value.c_str());
            else if (key == "server_name")
                server.serverName = value;
            else if (key == "client_max_body_size")
                server.clientMaxBodySize = atoi(value.c_str());
        }
        else if (line.find("error_page") == 0) {
            int code;
            std::string page;
            iss >> key >> code >> page;
            server.errorPage = page; // Simplified to store one error page
        }
    }
    throw std::runtime_error("Expected '}' to close server block");
}

void ConfigInterpreter::parseRouteBlock(ServerConfig& server, const std::vector<std::string>& lines, size_t& i)
{
    RouteConfig route;
    std::string header = trim(lines[i - 1]);
    std::istringstream iss(header);
    std::string dummy;
    iss >> dummy >> route.path;

    ++i;
    for (; i < lines.size(); ++i) {
        std::string line = trim(lines[i]);
        if (line.empty() || line[0] == '#') continue;

        if (line == "}") {
            server.routes.push_back(route);
            return;
        }

        if (line.find('=') != std::string::npos) {
            std::string key, value;
            std::istringstream iss(line);
            getline(iss, key, '=');
            getline(iss, value, ';');
            key = trim(key);
            value = trim(value);

            if (key == "methods") {
                std::istringstream methodStream(value);
                std::string method;
                while (methodStream >> method)
                    route.allowedMethods.push_back(method);
            }
            else if (key == "root") route.root = value;
            else if (key == "redirect") route.redirect = value;
            else if (key == "directory_listing") route.autoindex = (value == "on");
            else if (key == "index") route.indexFile = value;
            else if (key == "cgi") route.cgiExtension = value;
            else if (key == "upload_dir") route.uploadDir = value;
        }
    }
    throw std::runtime_error("Expected '}' to close route block");
}

std::vector<std::string> ConfigInterpreter::readFile(const std::string& filepath)
{
    std::ifstream file(filepath.c_str());
    if (!file.is_open())
        throw std::runtime_error("Cannot open config file");

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line)) {
        size_t commentPos = line.find('#');
        if (commentPos != std::string::npos)
            line = line.substr(0, commentPos);
        lines.push_back(trim(line));
    }
    return lines;
}

std::string ConfigInterpreter::trim(const std::string& line) {
    size_t start = line.find_first_not_of(" \t");
    size_t end = line.find_last_not_of(" \t");
    return (start == std::string::npos) ? "" : line.substr(start, end - start + 1);
}
