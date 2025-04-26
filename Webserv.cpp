/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/25 16:46:05 by ajabri            #+#    #+#             */
/*   Updated: 2025/04/26 11:25:49 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "webserv.hpp"


//*=====================GRAMMAR=================================
std::map<e_tokens, std::string> WebServ::GrammerMap;

void WebServ::initGrammerMap() {
    GrammerMap[TOKEN_SERVER_BLOCK] = "server";
    GrammerMap[TOKEN_LOCATION_BLOCK] = "location";
    GrammerMap[TOKEN_LISTEN] = "listen";
    GrammerMap[TOKEN_SERVER_NAME] = "server_name";
    GrammerMap[TOKEN_ERROR_PAGE] = "error_page";
    GrammerMap[TOKEN_CLIENT_MAX_BODY_SIZE] = "client_max_body_size";
    GrammerMap[TOKEN_ROOT] = "root";
    GrammerMap[TOKEN_INDEX] = "index";
    GrammerMap[TOKEN_METHODS] = "methods";
    GrammerMap[TOKEN_AUTOINDEX] = "autoindex";
    GrammerMap[TOKEN_UPLOAD] = "upload";
    GrammerMap[TOKEN_CGI_PASS] = "cgi_pass";
    GrammerMap[TOKEN_LBRACE] = "{";
    GrammerMap[TOKEN_RBRACE] = "}";
    GrammerMap[TOKEN_SEMICOLON] = ";";
    GrammerMap[TOKEN_UNKNOWN] = "unknown";
    GrammerMap[TOKEN_VALUE] = "value";
}

//*=====================CANONICAL=================================
WebServ::WebServ()
{
    m_FileName = "./config/default_config.config";
}

WebServ::WebServ(std::string config) {
    this->m_FileName = config;
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

//? this function is where i read config file parse it content and etc 
//*this is configParser entry point aka function
void WebServ::ReadConfig(std::fstream& configFile)
{
    std::string line;
    
    OpenConfigFile(configFile);
    while (std::getline(configFile, line)) {
        
        if (line.find('#') != std::string::npos || Isspaces(line)) //? this one here is to skip comment and empty line (lines that contain only spaces) in the config file
            continue;
        this->m_ConfigData.push_back(line);
    }
    //TODO 1
    //*i should build a lexer so i can give tokenize all configuration 
    //todo 2
    //* 
    for (size_t i = 0; i < this->m_ConfigData.size(); i++)
        ServerLogs(this->m_ConfigData[i]);
    initGrammerMap();
    for ( std::map<e_tokens, std::string>::iterator it = GrammerMap.begin(); it != GrammerMap.end(); it++)
    {
        // ServerLogs(GrammerMap[i].second);
        std::cout << "key: " << it->first << " value: " << it->second << std::endl;
    }
    
}
