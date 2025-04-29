#include "Webserver.hpp"

WebServ::WebServ()
{
    m_FileName = "./config/default_config.config";
    // initGrammerMap();
}

WebServ::WebServ(std::string config) : m_FileName(config), serverFlag(0), routeFlag(0)
{
    std::ifstream infile(config.c_str());
    if (!infile.is_open())
        throw std::runtime_error("Cannot open config file");

    std::string line;
    while (std::getline(infile, line))
    {
        m_ConfigData.push_back(line);
    }
    infile.close();
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


void WebServ::parseConfig()
{
    Server_block current_server;
    RouteConfig current_route;

    for (size_t i = 0; i < m_ConfigData.size(); ++i)
    {
        std::string line = trim(m_ConfigData[i]); //*remove spaces

        if (line.empty() || line[0] == '#') //* skip comments
            continue;

        if (line == "server {")
        {
            serverFlag = 1;
            continue;
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
            routeFlag = 1;
            current_route.path = extractPathFromRouteLine(line);
            continue;
        }

        if (serverFlag && !routeFlag)
        {
            parseServerLine(current_server, line);
        }
        else if (serverFlag && routeFlag)
        {
            parseRouteLine(current_route, line);
        }
    }
}

