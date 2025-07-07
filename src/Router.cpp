/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/30 09:02:04 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/07 12:12:00 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Router.hpp"
# include <iostream>
# include <string>

std::string clean_line(std::string line)
{
    // Trim leading and trailing whitespace
    line.erase(0, line.find_first_not_of(" \t\n\r"));
    line.erase(line.find_last_not_of(" \t\n\r") + 1);

    // Remove trailing semicolon if it exists
    if (!line.empty() && line.back() == ';')
    {
        line.pop_back();
    }
    return line;
}


// const RouteConfig* Router::match(const HttpRequest& request, const ServerConfig& serverConfig) const
// {
//     std::cout << "Matching route..." << std::endl;
//     const RouteConfig* match = 0;
//     const RouteConfig* defaultRoute = 0;
    
//     for (size_t j = 0; j < serverConfig.routes.size(); ++j) {
//         const std::string& routePath = serverConfig.routes[j].path;

//         if (!defaultRoute || routePath.length() < defaultRoute->path.length()) {
//             defaultRoute = &serverConfig.routes[j];
//         }

//         if (request.uri.find(routePath) == 0) {
//             if (!match || routePath.length() > match->path.length()) {
//                 match = &serverConfig.routes[j];
//             }
//         }
//     }

//     if (!match) {
//         // match = defaultRoute;
//         return NULL;
//     }

//     std::cout << "Matched route: " << (match ? match->path : "none") << std::endl;
//     return match;
// }

const RouteConfig* Router::match(const HttpRequest& request, const ServerConfig& serverConfig) const
{
    std::cout << "[Router] Matching route for URI: " << request.uri << std::endl;

    const RouteConfig* bestMatch = nullptr;

    for (std::vector<RouteConfig>::const_iterator it = serverConfig.routes.begin(); it != serverConfig.routes.end(); ++it) {
        const std::string& routePath = it->path;

        // Exact match or prefix match
        if (request.uri.compare(0, routePath.length(), routePath) == 0) {
            if (!bestMatch || routePath.length() > bestMatch->path.length()) {
                bestMatch = &(*it);
            }
        }
    }

    if (bestMatch) {
        std::cout << "[Router] Matched route: " << bestMatch->path << std::endl;
        return bestMatch;
    } else {
        std::cout << "[Router] No matching route found. Return nullptr." << std::endl;
        return nullptr; // tell dispatcher: no route → handle 404
    }
}
