/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GetHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/01 18:19:03 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/12 10:12:08 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

# include "IHttpMethodHandler.hpp"
# include "ServerConfig.hpp"


class GetHandler : public IHttpMethodHandler
{
    private:
        // Core functionality methods
        HttpResponse handleDirectory(const std::string& dirPath, const std::string& urlPath, bool listingEnabled, const ServerConfig& serverConfig) const;
        HttpResponse serveStaticFile(const std::string& filepath, const ServerConfig& serverConfig) const;
        HttpResponse handleDirectoryListing(const std::string& path, const std::string& urlPath, const ServerConfig& serverConfig) const;
        HttpResponse handleRedirect(const std::string& redirectUri) const;
        
        // Utility methods
        std::string getMimeType(const std::string& filePath) const;
        // std::string getParentPath(const std::string& urlPath) const; // ! remove this method later
        std::string buildLinkPath(const std::string& urlPath, const std::string& name) const;
        
        // Response builders
        HttpResponse createNotFoundResponse(const ServerConfig& serverConfig) const;
        HttpResponse createForbiddenResponse(const ServerConfig& serverConfig) const;
        HttpResponse createErrorResponse(int statusCode, const std::string& statusText, const ServerConfig& serverConfig) const;
        HttpResponse loadCustomErrorPage(int statusCode, const std::string& errorPagePath) const;
        
    public:
        GetHandler();
        ~GetHandler();
        // Legacy interface - uses default error pages
        // virtual HttpResponse handle(const HttpRequest &req, const RouteConfig& route) const;
        // New interface with ServerConfig for custom error pages
        virtual HttpResponse handle(const HttpRequest &req, const RouteConfig& route, const ServerConfig& serverConfig) const;
};
