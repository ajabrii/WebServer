/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/30 09:02:04 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/14 14:29:40 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Router.hpp"
#include "../includes/Utils.hpp"
# include <iostream>
# include <string>



const RouteConfig* Router::match(const HttpRequest& request, const ServerConfig& serverConfig) const
{
    std::cout << "[Router] Matching route for URI: " << request.uri << std::endl;

    const RouteConfig* bestMatch = 0;

    for (std::vector<RouteConfig>::const_iterator it = serverConfig.routes.begin(); it != serverConfig.routes.end(); ++it)
    {
        const std::string& routePath = it->path;

        if (request.uri.compare(0, routePath.length(), routePath) == 0)
        {
            if (!bestMatch || routePath.length() > bestMatch->path.length())
                bestMatch = &(*it);
        }
    }

    if (bestMatch) {
        std::cout << "[Router] Matched route: " << bestMatch->path << std::endl;
        return bestMatch;
    } else {
        std::cout << "[Router] No matching route found. Return nullptr." << std::endl;
        return 0;
    }
}
