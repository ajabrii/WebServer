/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestDispatcher.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/30 09:14:07 by ajabri            #+#    #+#             */
/*   Updated: 2025/06/30 09:17:10 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "../includes/RequestDispatcher.hpp"
# include <iostream>
HttpResponse RequestDispatcher::dispatch(const HttpRequest& request, const RouteConfig& route) const
{
    HttpResponse response;

    std:: cout << route.path << std::endl; // Accessing route path for potential logging or processing
    // Here we would typically call the appropriate handler based on the method
    // For now, let's assume we have a GetHandler that handles GET requests
    if (request.method == "GET") {
        // GetHandler getHandler;
        // response = getHandler.handle(request, route);
        response.statusCode = 200; // Placeholder for successful GET response
        response.body = "GET request handled successfully.";
    } else {
        response.statusCode = 405; // Method Not Allowed
        response.body = "Method not allowed.";
    }

    return response;
}
