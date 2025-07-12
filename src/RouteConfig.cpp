/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RouteConfig.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ytarhoua <ytarhoua@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 15:36:12 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/12 15:02:58 by ytarhoua         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "../includes/RouteConfig.hpp"

        RouteConfig::RouteConfig()
            : path(""),
              root(""),
              allowedMethods(),
              redirect(""),
              indexFile(""),
              cgi(),
              directory_listing(false),
              uploadDir("")
        {
        }