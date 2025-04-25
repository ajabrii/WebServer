/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/25 16:46:05 by ajabri            #+#    #+#             */
/*   Updated: 2025/04/25 17:14:36 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "webserv.hpp"

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
//*=====================CANONICAL=================================

void WebServ::ServerLogs(std::string logs)
{
    std::cerr << logs << std::endl;
}

std::string WebServ::GetFileName()
{
    return (m_FileName);
}
void WebServ::OpenConfigFile(std::fstream& configFile)
{
    configFile.open(this->m_FileName.c_str());
    if (!configFile.is_open()){
        WebServ::ServerLogs("Error: couldn't open config file");
    }
}