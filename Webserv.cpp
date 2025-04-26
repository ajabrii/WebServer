/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/25 16:46:05 by ajabri            #+#    #+#             */
/*   Updated: 2025/04/26 18:25:38 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "webserv.hpp"


//!=====================GRAMMAR=================================
// std::map<e_tokens, std::string> WebServ::GrammerMap;

// void WebServ::initGrammerMap() {
//     GrammerMap[TOKEN_SERVER_BLOCK] = "server";
//     GrammerMap[TOKEN_LOCATION_BLOCK] = "location";
//     GrammerMap[TOKEN_LISTEN] = "listen";
//     GrammerMap[TOKEN_SERVER_NAME] = "server_name";
//     GrammerMap[TOKEN_ERROR_PAGE] = "error_page";
//     GrammerMap[TOKEN_CLIENT_MAX_BODY_SIZE] = "client_max_body_size";
//     GrammerMap[TOKEN_ROOT] = "root";
//     GrammerMap[TOKEN_INDEX] = "index";
//     GrammerMap[TOKEN_METHODS] = "methods";
//     GrammerMap[TOKEN_AUTOINDEX] = "autoindex";
//     GrammerMap[TOKEN_UPLOAD] = "upload";
//     GrammerMap[TOKEN_CGI_PASS] = "cgi_pass";
//     GrammerMap[TOKEN_LBRACE] = "{";
//     GrammerMap[TOKEN_RBRACE] = "}";
//     GrammerMap[TOKEN_SEMICOLON] = ";";
//     GrammerMap[TOKEN_UNKNOWN] = "unknown";
//     GrammerMap[TOKEN_VALUE] = "value";
// }

//*=====================CANONICAL=================================
WebServ::WebServ()
{
    m_FileName = "./config/default_config.config";
    // initGrammerMap();
}

WebServ::WebServ(std::string config) {
    this->m_FileName = config;
    // initGrammerMap();

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
//*=====================getters/setters=================================

// std::vector<t_token> WebServ::GetTokens()
// {
//     return (m_Tokens);
// }

std::string WebServ::GetFileName()
{
    return (m_FileName);
}

std::vector<std::string> WebServ::GetConfigData()
{
    return m_ConfigData;    
}

//*=====================Parsing=================================
//? this function prints errors and logs in stderr
void WebServ::ServerLogs(std::string logs)
{
    std::cerr << logs << std::endl;
}


//? this function opens the config file so we can read it content
void WebServ::OpenConfigFile(std::fstream& configFile)
{
    configFile.open(this->m_FileName.c_str());
    if (!configFile.is_open()){
        WebServ::ServerLogs("Error: couldn't open config file");
    }
}

//*======================TOOLS-FUNCTIONS============================
//todo -> this function should be in the class or on it own file as a tool
bool Isspaces(const std::string& line) 
{
    for (size_t i = 0; i < line.length(); i++)
    {
        if (!std::isspace(line[i]))
            return false;
    }
    return true;
}
//? this one here is to skip comment and empty line (lines that contain only spaces) in the config file
bool IsComment(const std::string& line) 
{
    if (line.find('#') != std::string::npos || Isspaces(line))
    return true;
    return false;
}
//*========================/TOOL-FUNCTIONS============================
//? this function is where i read config file parse it content and etc 
//*this is configParser entry point aka function
void WebServ::ReadConfig(std::fstream& configFile)
{
    std::string line;
    std::string all_line;
    
    t_server_block serverBlock;
    OpenConfigFile(configFile);
    while (std::getline(configFile, line)) {
        
        if (IsComment(line)) //? this one here is to skip comment and empty line (lines that contain only spaces) in the config file
            continue;
        all_line.append(line);
        this->m_ConfigData.push_back(line);
    }
    
    ServerLogs(all_line);
    dataScraper(m_ConfigData);
    
}

void WebServ::dataScraper(std::vector<std::string> lines)
{
    for (size_t i = 0; i < lines.size(); i++)
    {
        if (lines[i].find("server") != std::string::npos)
        {
            
            t_server_block serverBlock;
            serverBlock.server_name = "default_server";
            serverBlock.listen = "80";
            serverBlock.error_page = "404";
            serverBlock.index = "index.html";
            serverBlock.client_max_body_size = "100M";
            this->m_ServerBlocks.push_back(serverBlock);
        }
        
    }
}







































//*======================printing==SERVER-BLOCK============================
// int i = 0;
// for (std::vector<std::string>::iterator it = this->m_ConfigData.begin() ; it != this->m_ConfigData.end(); it++)
// {
//     std::cout << "line: " << ++i << "  "<< *it << std::endl;
    
//     if (it->find("server") != std::string::npos)
//     {
//         serverBlock.server_name = "default_server";
//         serverBlock.listen = "80";
//         serverBlock.error_page = "404";
//         serverBlock.index = "index.html";
//         serverBlock.client_max_body_size = "100M";
//         this->m_ServerBlocks.push_back(serverBlock);
//     }
//     else if (it->find("location") != std::string::npos)
//     {
//         t_route_block routeBlock;
//         routeBlock.location = "/api";
//         routeBlock.methods = "GET, POST";
//         this->m_ServerBlocks.back().routes.push_back(routeBlock);
//     }
// }
// std::cout << "====================SERVER BLOCKS====================" << std::endl;
// for (size_t i = 0; i < this->m_ServerBlocks.size(); i++)
// {
//     std::cout << "server name: " << this->m_ServerBlocks[i].server_name << std::endl;
//     std::cout << "listen: " << this->m_ServerBlocks[i].listen << std::endl;
//     std::cout << "error page: " << this->m_ServerBlocks[i].error_page << std::endl;
//     std::cout << "index: " << this->m_ServerBlocks[i].index << std::endl;
//     std::cout << "client max body size: " << this->m_ServerBlocks[i].client_max_body_size << std::endl;
//     for (size_t j = 0; j < this->m_ServerBlocks[i].routes.size(); j++)
//     {
//         std::cout << "location: " << this->m_ServerBlocks[i].routes[j].location << std::endl;
//         std::cout << "methods: " << this->m_ServerBlocks[i].routes[j].methods << std::endl;
//     }
// }

//*======================printing=================================
//todo close file 
// for ( std::map<e_tokens, std::string>::iterator it = GrammerMap.begin(); it != GrammerMap.end(); it++)
// {
//     std::cout << "key: " << it->first << " value: " << it->second << std::endl;
// }
// for (size_t i = 0; i < this->m_ConfigData.size(); i++)
// {
//     //todo take line by line and assign it to a token
//     //todo: function