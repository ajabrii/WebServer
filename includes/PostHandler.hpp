/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PostHandler.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/01 18:16:22 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/01 18:25:42 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

# include "IHttpMethodHandler.hpp"

class PostHandler : public IHttpMethodHandler
{
    public:
        PostHandler();
        ~PostHandler();
        virtual HttpResponse &handle(const HttpRequest &req, RouteConfig *routeData) const;
};
