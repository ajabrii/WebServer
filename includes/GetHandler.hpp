/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GetHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/01 18:19:03 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/10 11:13:15 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

# include "IHttpMethodHandler.hpp"


class GetHandler : public IHttpMethodHandler
{
    private:
        // Core functionality methods
        HttpResponse handleDirectory(const std::string& dirPath, const std::string& urlPath, bool listingEnabled) const;
        HttpResponse serveStaticFile(const std::string& filepath) const;
        HttpResponse handleDirectoryListing(const std::string& path, const std::string& urlPath) const;
        HttpResponse handleRedirect(const std::string& redirectUri) const;
        
        // Utility methods
        std::string getMimeType(const std::string& filePath) const;
        std::string getParentPath(const std::string& urlPath) const;
        std::string buildLinkPath(const std::string& urlPath, const std::string& name) const;
        
        // Response builders
        HttpResponse createNotFoundResponse() const;
        HttpResponse createForbiddenResponse() const;
        
    public:
        GetHandler();
        ~GetHandler();
        virtual HttpResponse handle(const HttpRequest &req, const RouteConfig& route) const;
};
