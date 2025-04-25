/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/25 16:46:05 by ajabri            #+#    #+#             */
/*   Updated: 2025/04/25 19:52:12 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "webserv.hpp"


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

void WebServ::ReadConfig(std::fstream& configFile)
{
    std::string line;
    
    OpenConfigFile(configFile);
    while (std::getline(configFile, line)) {
        
        if (line.find('#') != std::string::npos) //? this one here is to skip comment in the config file
            continue;
        this->m_ConfigData.push_back(line);
    }
    for (size_t i = 0; i < this->m_ConfigData.size(); i++)
        ServerLogs(this->m_ConfigData[i]);
    
}
