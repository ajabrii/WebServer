/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ytarhoua <ytarhoua@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/30 08:58:47 by ajabri            #+#    #+#             */
/*   Updated: 2025/06/30 11:04:54 by ytarhoua         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "ServerConfig.hpp"
# include "RouteConfig.hpp"
# include "HttpServer.hpp"


class Router
{
    public:
         const RouteConfig* match(HttpRequest& request, ServerConfig& serverConfig) const;
};