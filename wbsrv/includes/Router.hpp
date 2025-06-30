/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/30 08:58:47 by ajabri            #+#    #+#             */
/*   Updated: 2025/06/30 09:01:42 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "ServerConfig.hpp"
# include "RouteConfig.hpp"
# include "HttpServer.hpp"


class Router
{
    public:
         const RouteConfig* match(const HttpRequest& request, const ServerConfig& serverConfig) const;
};