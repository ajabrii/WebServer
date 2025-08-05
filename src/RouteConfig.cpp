/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RouteConfig.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 15:36:12 by ajabri            #+#    #+#             */
/*   Updated: 2025/08/02 09:54:44 by ajabri           ###   ########.fr       */
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