/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DeleteHandler.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/01 18:19:20 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/01 18:35:21 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

# include "IHttpMethodHandler.hpp"

class DeleteHandler : public IHttpMethodHandler
{
    public:
        DeleteHandler();
        ~DeleteHandler();
        virtual HttpResponse &handle(const HttpRequest &req, const RouteConfig& route) const;
};
