/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RouteConfig.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: youness <youness@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 15:36:12 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/08 22:35:26 by youness          ###   ########.fr       */
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
              uploadPath("")
        {
        }