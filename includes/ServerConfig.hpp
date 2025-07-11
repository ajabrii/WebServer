/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: youness <youness@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 15:00:24 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/10 18:45:33 by youness          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_CONFIG_HPP
#define SERVER_CONFIG_HPP

#include <string>
#include <vector>
#include <map>
#include "RouteConfig.hpp"

class ServerConfig {
    public:
        std::string host;
        std::vector<int> port;
        // int port;
        std::vector<std::string> serverName;
        std::map<int, std::string> error_pages;
        size_t clientMaxBodySize;
        std::vector<RouteConfig> routes;
        
        ServerConfig();
        ServerConfig(const ServerConfig& other);
        int getMaxSize();
        ServerConfig& operator=(const ServerConfig& other);
};

#endif

