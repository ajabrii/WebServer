/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: youness <youness@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 15:00:24 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/30 17:08:08 by youness          ###   ########.fr       */
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
        std::vector<std::string> serverName;
        std::map<int, std::string> error_pages;
        unsigned long long clientMaxBodySize;
        bool clientMaxBodySizeSet;  // Flag to track if explicitly set
        std::vector<RouteConfig> routes;
        
        ServerConfig();
        ServerConfig(const ServerConfig& other);
        int getMaxSize();
        ServerConfig& operator=(const ServerConfig& other);
};

#endif

