/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GetHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/01 18:19:03 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/02 11:40:29 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

# include "IHttpMethodHandler.hpp"


class GetHandler : public IHttpMethodHandler
{
    private:
        HttpResponse serveStaticFile(std::string& filepath) const;
        HttpResponse  handleDirectoryListing(const std::string& path, const std::string& urlPath) const;
        HttpResponse handleRedirect(const std::string& redirectUri) const;
    public:
        GetHandler();
        ~GetHandler();
        virtual HttpResponse handle(const HttpRequest &req, const RouteConfig& route) const;
};
