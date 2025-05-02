#include "Webserver.hpp"

WebServ::WebServ()
{
    m_FileName = "./config/default_config.config";
    
}

WebServ::WebServ(std::string config) : m_FileName(config), serverFlag(0), routeFlag(0)
{
    std::string tmp;
    size_t pos;
    std::cout << m_FileName << "\n";
    std::ifstream infile(config.c_str());
    if (!infile.is_open())
        throw std::runtime_error("Cannot open config file");

    std::string line;
    while (std::getline(infile, line))
    {
        // std::cout << "std::line :: " << line << "\n"; 
        pos = line.find('{');
        if (pos != std::string::npos && pos != 0) {
            tmp = line.substr(0, pos);
            this->m_ConfigData.push_back(trim(tmp));
            tmp = line.substr(pos, line.length() - 1);
            this->m_ConfigData.push_back(trim(tmp));
            continue;
        }
        m_ConfigData.push_back(trim(line));
    }
    // for (size_t i = 0; i < m_ConfigData.size(); ++i)
    // {
    //     std::cout << "std::line :: " << m_ConfigData[i] << "\n";
    // }
    infile.close();
}

std::vector<Server_block> WebServ::getServerBlocks() const
{
    return m_ServerBlocks;
}

WebServ::WebServ(const WebServ& other)
{
    *this = other; //TODO
}

WebServ& WebServ::operator=(const WebServ& other)
{
    //todo
   (void)other;
    return *this;
}

WebServ::~WebServ()
{
}

bool Isspaces(const std::string& line) 
{
    for (size_t i = 0; i < line.length(); i++)
    {
        if (!std::isspace(line[i]))
            return false;
    }
    return true;
}

bool IsComment(const std::string& line) 
{
    //! handle comment in the same line ex: error_page 404 = /404.html; #comment 
    if (line.empty() || line.find('#') != std::string::npos || Isspaces(line))
        return true;
    return false;
    
}

void WebServ::parseConfig()
{
    Server_block current_server;
    RouteConfig current_route;
    std::string line;

    for (size_t i = 0; i < m_ConfigData.size(); i++)
    {
        line = m_ConfigData[i];
        if (IsComment(line)) //* skip comments
            continue;
        if (line == "server")
        {
            if (m_ConfigData[i+1] == "{")
            {
                serverFlag = 1;
                i++;
                continue;
            }
        }
        if (line == "}" && routeFlag)
        {
            // End of route block
            current_server.routes.push_back(current_route);
            current_route = RouteConfig(); // reset
            routeFlag = 0;
            continue;
        }
        else if (line == "}" && serverFlag)
        {
            // End of server block
            m_ServerBlocks.push_back(current_server);
            current_server = Server_block(); // reset
            serverFlag = 0;
            continue;
        }

        if (line.substr(0, 5) == "route")
        {
            if (m_ConfigData[i+1] == "{")
            {
                // std::cout << "Server block found\n";
                current_route.path = extractPathFromRouteLine(line);
                routeFlag = 1;
                i++;
                continue;
            }
            continue;
        }

        if (serverFlag && !routeFlag)
        {
            // std::cout << i << " : " << line << "\n";
            parseServerLine(current_server, line);
        }
        else if (serverFlag && routeFlag)
        {
            parseRouteLine(current_route, line);
        }
    }
}

