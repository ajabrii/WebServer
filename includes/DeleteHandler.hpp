/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DeleteHandler.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/01 18:19:20 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/10 16:22:02 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

# include "IHttpMethodHandler.hpp"

class DeleteHandler : public IHttpMethodHandler
{
    private:
        // Helper methods for file operations
        std::string buildFilePath(const std::string& root, const std::string& requestPath) const;
        bool isPathSecure(const std::string& filePath, const std::string& rootPath) const;
        
        // Response creation methods
        HttpResponse createSuccessResponse(const std::string& filePath, const ServerConfig& serverConfig) const;
        HttpResponse createErrorResponse(int statusCode, const std::string& statusText, 
                                       const std::string& message, const ServerConfig& serverConfig) const;
        HttpResponse createRedirectResponse(const std::string& redirectUrl, const ServerConfig& serverConfig) const;
        
        // Utility methods
        std::string getCurrentTimestamp() const;
        
    public:
        DeleteHandler();
        ~DeleteHandler();
        // Legacy interface
        virtual HttpResponse handle(const HttpRequest &req, const RouteConfig& route) const;
        // New interface with ServerConfig
        virtual HttpResponse handle(const HttpRequest &req, const RouteConfig& route, const ServerConfig& serverConfig) const;
};
