/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DeleteHandler.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/01 18:19:20 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/22 12:48:29 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

# include "IHttpMethodHandler.hpp"

class DeleteHandler : public IHttpMethodHandler
{
    private:
        std::string buildFilePath(const std::string& root, const std::string& requestPath) const;
        bool isPathSecure(const std::string& filePath, const std::string& rootPath) const;

        HttpResponse createSuccessResponse(const std::string& filePath) const;
        HttpResponse createErrorResponse(int statusCode, const std::string& statusText,const ServerConfig& serverConfig) const;
        HttpResponse createRedirectResponse(const std::string& redirectUrl) const;

        std::string getCurrentTimestamp() const;

    public:
        DeleteHandler();
        ~DeleteHandler();

        virtual HttpResponse handle(const HttpRequest &req, const RouteConfig& route, const ServerConfig& serverConfig) const;
};
