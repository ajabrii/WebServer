/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IHttpMethodHandler.hpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/01 18:11:07 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/02 11:40:22 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "HttpResponse.hpp"
#include "HttpRequest.hpp"
#include "RouteConfig.hpp"

class IHttpMethodHandler
{
    
    public:
        virtual ~IHttpMethodHandler(){};
        virtual HttpResponse handle(const HttpRequest& req, const RouteConfig& route) const = 0;
};
