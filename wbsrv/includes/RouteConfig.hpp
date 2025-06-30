/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RouteConfig.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ytarhoua <ytarhoua@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 14:59:00 by ajabri            #+#    #+#             */
/*   Updated: 2025/06/30 11:08:45 by ytarhoua         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ROUTE_CONFIG_HPP
#define ROUTE_CONFIG_HPP

#include <string>
#include <vector>

class RouteConfig {
    public:
        std::string path;
        std::string root;
        std::vector<std::string> allowedMethods;
        std::string redirect;
        bool autoindex;
        std::string indexFile;
        std::string cgiExtension;
        std::string uploadDir;
        bool directory_listing;
        
        RouteConfig();
};

#endif
