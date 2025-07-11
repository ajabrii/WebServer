/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: youness <youness@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 15:35:32 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/10 18:45:40 by youness          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "../includes/ServerConfig.hpp"

ServerConfig::ServerConfig(){

    host = "";
    // port[0] = 0;
    clientMaxBodySize = 1024 * 1024; // Default to 1MB
    // server_names.clear();
    // error_pages.clear();
    // routes.clear();
}

ServerConfig::ServerConfig(const ServerConfig& other) {

    // if (this == other)
    this->clientMaxBodySize = other.clientMaxBodySize;
    this->error_pages = other.error_pages;
    this->host = other.host;
    this->port = other.port;
    this->routes = other.routes;
    this->serverName = other.serverName;
}

ServerConfig& ServerConfig::operator=(const ServerConfig& other)
{
    this->clientMaxBodySize = other.clientMaxBodySize;
    this->error_pages = other.error_pages;
    this->host = other.host;
    this->port = other.port;
    this->routes = other.routes;
    this->serverName = other.serverName;
    return *this;
}

int ServerConfig::getMaxSize(){
    return this->clientMaxBodySize;
}