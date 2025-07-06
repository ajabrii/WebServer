/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RouteConfig.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ytarhoua <ytarhoua@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 14:59:00 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/05 21:02:45 by ytarhoua         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ROUTE_CONFIG_HPP
#define ROUTE_CONFIG_HPP

#include <string>
#include <vector>
#include <map>

class RouteConfig {
    public:
        std::string path;
        std::string root;
        std::vector<std::string> allowedMethods;
        std::string redirect;
        // bool autoindex;
        std::string indexFile;
        std::map<std::string, std::string> cgi;
        // std::string interpath;
        bool directory_listing;
        std::string uploadDir;
        
        RouteConfig();
};

#endif
