/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 15:00:24 by ajabri            #+#    #+#             */
/*   Updated: 2025/06/26 15:58:34 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_CONFIG_HPP
#define SERVER_CONFIG_HPP

// #include <string>
// #include <vector>
#include "RouteConfig.hpp"

class ServerConfig {
    public:
        std::string host;
        int port;
        std::string serverName;
        std::string errorPage;
        size_t clientMaxBodySize;
        std::vector<RouteConfig> routes;
        
        ServerConfig();
};

#endif
