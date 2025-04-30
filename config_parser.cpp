#include "Webserver.hpp"
#include <sstream>

std::string WebServ::trim(const std::string& str)
{
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos)
        return "";

    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

std::string WebServ::extractPathFromRouteLine(const std::string& line) {
    size_t start = 0;
    std::string path;

    if ((line.find("route")) != std::string::npos) {
        start = line.find("route") + 5;
    } else if ((line.find("location")) != std::string::npos) {
        start = line.find("location") + 8;
    }

    size_t end = line.find("{"); //? i will change later
    if (end <= start)
        throw std::runtime_error("Invalid route line: " + line);

    path = line.substr(start, end - start);
    return trim(path);
}


void WebServ::parseServerLine(Server_block& server, const std::string& line)
{
    size_t equal = line.find('=');
    if (equal == std::string::npos && line.find("error_page") != 0)
        throw std::runtime_error("Invalid server line: " + line);

    std::string key = trim(line.substr(0, equal));
    std::string value = trim(line.substr(equal + 1));

    if (key == "host")
        server.host = value;
    else if (key == "port")
        server.port = std::atoi(value.c_str());
    else if (key == "server_name")
    {
        std::stringstream ss(value);
        std::string name;
        while (ss >> name)
            server.server_names.push_back(name);
    }
    else if (key == "client_max_body_size")
    {
        if (value.back() == 'M') {
            value.pop_back(); //* remove 'M'
            server.client_max_body_size = std::atoi(value.c_str()) * 1024 * 1024; //? if size in megabyts
        } else
            server.client_max_body_size = std::atoi(value.c_str()); //? if i set default in byts
    }
    else if (key.find("error_page") == 0)
    {
        std::stringstream ss(line);
        std::string temp;
        int code;
        std::string path;
        ss >> temp;
        ss >> code;
        ss >> path;
        server.error_pages[code] = path;
    }
    else
        throw std::runtime_error("Unknown server option: " + key);
}

void WebServ::parseRouteLine(RouteConfig& route, const std::string& line)
{
    // std::cout << "--------------------------------------------->line: " << line << "\n";
    size_t equal = line.find('=');
    if (equal == std::string::npos)
        throw std::runtime_error("Invalid route line: " + line);

    std::string key = trim(line.substr(0, equal));
    std::string value = trim(line.substr(equal + 1));

    if (key == "methods")
    {
        std::stringstream ss(value);
        std::string method;
        while (ss >> method)
            route.methods.push_back(method);
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
    else
        throw std::runtime_error("Unknown route option: " + key);
}


void WebServ::checkValues() const
{
    for (size_t i = 0; i < m_ServerBlocks.size(); i++)
    {
        if (m_ServerBlocks[i].host.empty())
            throw std::runtime_error("Host is not set for a server block.");
        if (m_ServerBlocks[i].port == 0)
            throw std::runtime_error("Port is not set for a server block.");
        for (size_t j = 0; j < m_ServerBlocks[i].routes.size(); j++)
        {
            if (m_ServerBlocks[i].routes[j].path.empty())
                throw std::runtime_error("Route path is not set.");
            if (m_ServerBlocks[i].routes[j].methods.empty())
                throw std::runtime_error("No methods specified for route: " + m_ServerBlocks[i].routes[j].path);
        }
    }
}

void WebServ::printConfig() const
{
    // Iterate over each serve
    for (const auto& server : m_ServerBlocks)
    {
        std::cout << "Server Configuration:\n";
        std::cout << "  Host: " << server.host << "\n";
        std::cout << "  Port: " << server.port << "\n";
        
        // Print server names
        std::cout << "  Server Names: ";
        for (const auto& name : server.server_names)
            std::cout << name << " ";
        std::cout << "\n";

        // Print client max body size
        std::cout << "  Client Max Body Size: " << server.client_max_body_size << " bytes\n";

        // Print error pages
        std::cout << "  Error Pages:\n";
        for (const auto& error : server.error_pages)
        {
            std::cout << "    Error " << error.first << " -> " << error.second << "\n";
        }

        // Print routes
        std::cout << "  Routes:\n";
        for (const auto& route : server.routes)
        {
            std::cout << "    Path: " << route.path << "\n";

            std::cout << "    Methods: ";
            for (const auto& method : route.methods)
                std::cout << method << " ";
            std::cout << "\n";

            std::cout << "    Root: " << route.root << "\n";

            std::cout << "    Directory Listing: " << (route.directory_listing ? "On" : "Off") << "\n";

            if (!route.redirect.empty())
                std::cout << "    Redirect: " << route.redirect << "\n";
        }

        std::cout << "\n---------------------------------\n";
    }
}
