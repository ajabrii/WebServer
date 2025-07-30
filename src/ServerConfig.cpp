/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: youness <youness@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 15:35:32 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/30 17:12:01 by youness          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "../includes/ServerConfig.hpp"

ServerConfig::ServerConfig() : clientMaxBodySize(1024 * 1024), clientMaxBodySizeSet(false) {
}

ServerConfig::ServerConfig(const ServerConfig& other) {

    // if (this == other)
    this->clientMaxBodySize = other.clientMaxBodySize;
    this->clientMaxBodySizeSet = other.clientMaxBodySize;
    this->error_pages = other.error_pages;
    this->host = other.host;
    this->port = other.port;
    this->routes = other.routes;
    this->serverName = other.serverName;
}

ServerConfig& ServerConfig::operator=(const ServerConfig& other)
{
    if (this != &other) {
        host = other.host;
        port = other.port;
        clientMaxBodySize = other.clientMaxBodySize;
        clientMaxBodySizeSet = other.clientMaxBodySizeSet;
        serverName = other.serverName;
        error_pages = other.error_pages;
        routes = other.routes;
    }
    return *this;
}

int ServerConfig::getMaxSize(){
    return this->clientMaxBodySize;
}