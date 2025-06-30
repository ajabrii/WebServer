/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/30 09:02:04 by ajabri            #+#    #+#             */
/*   Updated: 2025/06/30 09:38:50 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Router.hpp"
# include <iostream>
# include <string>

const RouteConfig* Router::match(const HttpRequest& request, const ServerConfig& serverConfig) const
{
    (void)request; // Avoid unused parameter warning
    (void)serverConfig; // Avoid unused parameter warning
    // for (size_t i = 0; i < serverConfig.routes.size(); ++i)
    // {
    //     const RouteConfig& route = serverConfig.routes[i];
    //     // Check if the request URI matches the route path
    //     if (request.uri.find(route.path) == 0) // Simple prefix match
    //     {
    //         // Check if the method is allowed
    //         if (std::find(route.allowedMethods.begin(), route.allowedMethods.end(), request.method) != route.allowedMethods.end())
    //         {
    //             return &route; // Return the matched route
    //         }
    //     }
    // }

    return 0;
}
