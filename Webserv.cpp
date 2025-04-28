/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/25 16:46:05 by ajabri            #+#    #+#             */
/*   Updated: 2025/04/27 18:35:42 by ajabri           ###   ########.fr       */
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

// void  fixVecotrData(std::vector<std::string>& lines)
// {

// }

//*========================/TOOL-FUNCTIONS============================
//? this function is where i read config file parse it content and etc 
//*this is configParser entry point aka function

void WebServ::ReadConfig(std::fstream& configFile)
{
    std::string line;
    std::string all_line;
    std::string tmp;
    size_t pos;
    
    t_server_block serverBlock;
    OpenConfigFile(configFile);
    while (std::getline(configFile, line)) {
        
        if (IsComment(line))
            continue;
        //? youness is this is where fix this error server { and server \n{
        pos = line.find('{');
        if (pos != std::string::npos && pos != 0) {
            tmp = line.substr(0, pos);
            this->m_ConfigData.push_back(Trim(tmp));
            tmp = line.substr(pos, line.length() - 1);
            this->m_ConfigData.push_back(Trim(tmp));
            continue;
        }
        else
            this->m_ConfigData.push_back(Trim(line));
    }
    dataScraper(m_ConfigData);
}

std::string WebServ::Trim(std::string& str)
{
      size_t first = 0;
    while (first < str.length() && isspace(str[first])) {
        first++;
    }
    
    size_t last = str.length();
    while (last > first && isspace(str[last - 1])) {
        last--;
    }
    
    return str.substr(first, last - first);
}

void WebServ::getServerData(std::string& line, t_server_block& block) // host = 0.0.0.0
{
    size_t pos;
    pos = line.find('=');
    std::string key = line.substr(0, pos - 1);
    key = Trim(key);
    std::string Value = line.substr(pos + 1, line.length() - 1);
    Value = Trim(Value);
    if (key == "host")
        block.host = Value;
    else if (key == "port")
        block.listen = Value;
    else if (key == "server_name")
        block.server_name = Value;
    else if (key == "error_page")
        block.error_page = Value;
    else if (key == "client_max_body_size")
        block.client_max_body_size = Value;
    else {
        std::cerr << "Error: unknown key " << key << std::endl;
        return;
    }
    
}

void WebServ::getRouteData(std::string& line, t_server_block& block)
{
    (void)block;
    // size_t pos;
    if (line.find("route") == std::string::npos) {
        ServerLogs("Error: invalid line format " + line);
        //exit(1);
        return;
    }
}

void WebServ::ServerData(std::vector<std::string> &lines, size_t& i)
{
    t_server_block serverBlock;

    while (i < lines.size()) {
        if (lines[i] == "}") {
            serverFlag = 0;
            break;
        }
        if (lines[i].find('=') == std::string::npos) {
            getRouteData(lines[i],serverBlock);
            i++;
            
        }
        getServerData(lines[i],serverBlock);
        i++;
    }
    // std::cout << "====================SERVER BLOCKS====================" << std::endl;
    m_ServerBlocks.push_back(serverBlock);
    // std::cout << "server name: " << serverBlock.server_name << std::endl;
    // std::cout << "port: " << serverBlock.listen << std::endl;
    // std::cout << "host: " << serverBlock.host << std::endl;
    // std::cout << "error page: " << serverBlock.error_page << std::endl;
    // std::cout << "client max body size: " << serverBlock.client_max_body_size << std::endl;
    
}

void WebServ::dataScraper(std::vector<std::string> &lines)
{
    serverFlag = -1;
    for (size_t i = 0; i < lines.size(); i++)
    {
        if (lines[i] == "server")
        {
            if (lines[i+1] == "{")
            {
                serverFlag = 1;
                ServerData(m_ConfigData, i += 2);  
            }
            if (serverFlag != 0)
            {
                ServerLogs("Error: server block not closed");
            }
        
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