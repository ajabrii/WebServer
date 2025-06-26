/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 15:35:32 by ajabri            #+#    #+#             */
/*   Updated: 2025/06/26 17:16:30 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "../includes/ServerConfig.hpp"

ServerConfig::ServerConfig(){}

ServerConfig::ServerConfig(const ServerConfig& other) {

    // if (this == other)
    this->clientMaxBodySize = other.clientMaxBodySize;
    this->errorPage = other.errorPage;
    this->host = other.host;
    this->port = other.port;
    this->routes = other.routes;
    this->serverName = other.serverName;
}