#include "Webserver.hpp"

WebServ::WebServ()
{
    m_FileName = "./config/default_config.config";
}

WebServ::WebServ(std::string config) : m_FileName(config), serverFlag(0), routeFlag(0)
{
    std::string line;
    size_t commentPos;
    std::vector<std::string> blockKeywords = {"server", "location", "route"};
    bool matched;
    std::string keyword;
    // std::cout << m_FileName << "\n";
    std::ifstream infile(config.c_str());
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
                m_ConfigData.push_back(keyword);
                matched = true;
                break;
            }

            //? handell this problem :: "keyword {" or "keyword  \n {"
            if (line.find(keyword) != std::string::npos && line.find("{") != std::string::npos) {
                size_t keyPos = line.find(keyword);
                size_t bracePos = line.find("{");

                if (keyPos < bracePos) {
                    m_ConfigData.push_back(trim(line.substr(0, bracePos)));
                    m_ConfigData.push_back("{");
                    matched = true;
                    break;
                }
            }
        }

        if (!matched) {
            m_ConfigData.push_back(line);
        }
    }
    // for (size_t i = 0; i < m_ConfigData.size(); ++i)
    // {
    //     std::cout << "std::line :: " << m_ConfigData[i] << "\n";
    // }
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

bool WebServ::Isspaces(const std::string& line) 
{
    for (size_t i = 0; i < line.length(); i++)
    {
        if (!std::isspace(line[i]))
            return false;
    }
    return true;
}

bool WebServ:: IsComment(const std::string& line) 
{
    //! handle comment in the same line ex: error_page 404 = /404.html; #comment 
    //? done
    if (line.empty() || line.find('#') != std::string::npos || Isspaces(line))
        return true;
    return false;
}

void WebServ::parseConfig()
{
    Server_block current_server;
    RouteConfig current_route;
    std::string line;

    // for (size_t i = 0; i < m_ConfigData.size(); i++)
    // {
    //     std::cout << "std::line :: " << m_ConfigData[i] << "\n";
    // }
    for (size_t i = 0; i < m_ConfigData.size(); i++)
    {
        line = m_ConfigData[i];
        if (IsComment(line)) 
            continue;

        if (line == "server")
        {
            if (i + 1 < m_ConfigData.size() && m_ConfigData[i + 1] == "{")
            {
                std::cout << "Server block found\n";
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
                m_ServerBlocks.push_back(current_server);
                current_server = Server_block();
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
            if (i + 1 < m_ConfigData.size() && m_ConfigData[i + 1] == "{")
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
            {
                parseRouteLine(current_route, line);
            }
            else
            {
                parseServerLine(current_server, line);
            }
        }
    }
}


// void WebServ::parseConfig()
// {
//     Server_block current_server;
//     RouteConfig current_route;
//     std::string line;

//     for (size_t i = 0; i < m_ConfigData.size(); ++i)
//     {
//         std::cout << "std::line :: " << m_ConfigData[i] << "\n";
//     }
//     for (size_t i = 0; i < m_ConfigData.size(); i++)
//     {
//         line = m_ConfigData[i];
//         if (IsComment(line)) //* skip comments
//             continue;
//         if (line == "server")
//         {
//             if (m_ConfigData[i+1] == "{")
//             {
//                 std::cout << "Server block found\n";
//                 serverFlag = 1;
//                 i++;
//                 continue;
//             }
//             else
//                 throw std::runtime_error("Invalid server block syntax");
//         }
//         if (line == "}" && routeFlag)
//         {
//             // End of route block
//             current_server.routes.push_back(current_route);
//             current_route = RouteConfig(); // reset
//             routeFlag = 0;
//             continue;
//         }
//         else if (line == "}" && serverFlag)
//         {
//             // End of server block
//             m_ServerBlocks.push_back(current_server);
//             current_server = Server_block(); // reset
//             serverFlag = 0;
//             continue;
//         }
//         if (line == "}" && !serverFlag && !routeFlag)
//         {
//             throw std::runtime_error("Invalid config file syntax");
//         }
//         if (line == "{" && !serverFlag && !routeFlag)
//         {
//             throw std::runtime_error("Invalid config file syntax");
//         }
//         if (line == "{" && serverFlag && !routeFlag)
//         {
//             throw std::runtime_error("Invalid config file syntax");
//         }
//         if (line == "{" && serverFlag && routeFlag)
//         {
//             throw std::runtime_error("Invalid config file syntax");
//         }

//         if (line.substr(0, 5) == "route" || line.substr(0, 8) == "location")
//         {
//             if (m_ConfigData[i+1] == "{")
//             {
//                 // std::cout << "Server block found\n";
//                 current_route.path = extractPathFromRouteLine(line);
//                 routeFlag = 1;
//                 i++;
//                 continue;
//             }
//             else
//                 throw std::runtime_error("Invalid route block syntax");
//         }

//         if (serverFlag && !routeFlag)
//         {
            
//             parseServerLine(current_server, line);
//         }
//         else if (serverFlag && routeFlag)
//         {
//             parseRouteLine(current_route, line);
//         }
//     }
// }

