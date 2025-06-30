/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestDispatcher.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/30 09:13:08 by ajabri            #+#    #+#             */
/*   Updated: 2025/06/30 09:13:31 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_DISPATCHER_HPP
#define REQUEST_DISPATCHER_HPP

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "RouteConfig.hpp"
// #include "GetHandler.hpp"
// + PostHandler.hpp, DeleteHandler.hpp when implemented

class RequestDispatcher {
public:
    HttpResponse dispatch(const HttpRequest& request, const RouteConfig& route) const;
};

#endif
