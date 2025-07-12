/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestDispatcher.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/30 09:13:08 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/10 13:36:19 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_DISPATCHER_HPP
#define REQUEST_DISPATCHER_HPP

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "RouteConfig.hpp"
#include "ServerConfig.hpp"
// #include "GetHandler.hpp"
// + PostHandler.hpp, DeleteHandler.hpp when implemented

class RequestDispatcher
{
    // private:
    //     HttpResponse serveStaticFile(std::string& filepath) const;
    //     HttpResponse  handleDirectoryListing(const std::string& path, const std::string& urlPath) const;
    //     HttpResponse handleRedirect(const std::string& redirectUri) const;
    public:
        HttpResponse dispatch(const HttpRequest& request, const RouteConfig& route, const ServerConfig& serverConfig) const;
};

#endif
