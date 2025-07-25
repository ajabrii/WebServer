/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PostHandler.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/01 18:16:22 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/13 20:37:52 by ajabri           ###   ########.fr       */
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
        virtual HttpResponse handle(const HttpRequest &req, const RouteConfig& route, const ServerConfig& serverConfig) const;
};
