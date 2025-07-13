/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GetHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/01 18:19:03 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/13 19:46:35 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

# include "IHttpMethodHandler.hpp"
# include "ServerConfig.hpp"

# define REDIRCT_LOG  "\033[1;33m[GET Handler]\033[0m Redirect configured to: "
#define URI_PROCESS_LOG "\033[1;34m[GET Handler]\033[0m Processing request for URI: "
# define STATIC_FILE_LOG "\033[1;32m[GET Handler]\033[0m Attempting to serve file: '"
#define DIRECTORY_LISTING_LOG "\033[1;32m[GET Handler]\033[0m Generating directory listing for: "

class GetHandler : public IHttpMethodHandler
{
    private:
        HttpResponse handleDirectory(const std::string& dirPath, const std::string& urlPath, bool listingEnabled, const ServerConfig& serverConfig) const;
        HttpResponse serveStaticFile(const std::string& filepath, const ServerConfig& serverConfig) const;
        HttpResponse handleDirectoryListing(const std::string& path, const std::string& urlPath, const ServerConfig& serverConfig) const;
        HttpResponse handleRedirect(const std::string& redirectUri) const;

        std::string getMimeType(const std::string& filePath) const;
        std::string buildLinkPath(const std::string& urlPath, const std::string& name) const;

        HttpResponse createNotFoundResponse(const ServerConfig& serverConfig) const;
        HttpResponse createForbiddenResponse(const ServerConfig& serverConfig) const;
        HttpResponse createErrorResponse(int statusCode, const std::string& statusText, const ServerConfig& serverConfig) const;

    public:
        GetHandler();
        ~GetHandler();
        virtual HttpResponse handle(const HttpRequest &req, const RouteConfig& route, const ServerConfig& serverConfig) const;
};
