/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RouteConfig.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 14:59:00 by ajabri            #+#    #+#             */
/*   Updated: 2025/06/26 14:59:35 by ajabri           ###   ########.fr       */
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
        std::string redirection;
        bool autoindex;
        std::string indexFile;
        std::string cgiExtension;
        std::string uploadDir;
        
        RouteConfig();
};

#endif
