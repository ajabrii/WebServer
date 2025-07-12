/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PostHandler.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/01 18:16:22 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/12 10:26:47 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

# include "IHttpMethodHandler.hpp"
struct Part {
    std::string name;
    std::string filename;
    std::string content;
};

class PostHandler : public IHttpMethodHandler
{
    public:
        PostHandler();
        ~PostHandler();
        // Legacy interface
        virtual HttpResponse handle(const HttpRequest &req, const RouteConfig& route) const;
        // New interface with ServerConfig
        virtual HttpResponse handle(const HttpRequest &req, const RouteConfig& route, const ServerConfig& serverConfig) const;
        // std::string extractBoundary(const std::string& contentType);
        // std::map<std::string, std::string> parseFormUrlEncoded(const std::string& body);
        // void parseAndSaveFile(const std::string& body, const std::string& boundary, const std::string& uploadDir);
};
