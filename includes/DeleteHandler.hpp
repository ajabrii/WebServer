/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DeleteHandler.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/01 18:19:20 by ajabri            #+#    #+#             */
/*   Updated: 2025/08/06 12:29:02 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "IHttpMethodHandler.hpp"

class DeleteHandler : public IHttpMethodHandler
{
private:
    std::string urlDecode(const std::string &encoded) const;
    std::string buildFilePath(const std::string &root, const std::string &requestPath) const;
    bool isPathSecure(const std::string &filePath, const std::string &rootPath) const;

public:
    DeleteHandler();
    ~DeleteHandler();

    virtual HttpResponse handle(const HttpRequest &req, const RouteConfig &route, const ServerConfig &serverConfig) const;
};
