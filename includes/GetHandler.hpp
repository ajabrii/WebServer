/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GetHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/01 18:19:03 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/01 18:21:52 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

# include "IHttpMethodHandler.hpp"

class GetHandler : public IHttpMethodHandler
{
    public:
        GetHandler();
        ~GetHandler();
        virtual HttpResponse &handle(const HttpRequest &req, RouteConfig *routeData) const;
};
