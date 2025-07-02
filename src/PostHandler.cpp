/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PostHandler.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/01 18:26:13 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/02 11:37:22 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


# include "../includes/PostHandler.hpp"

PostHandler::PostHandler(){ }

PostHandler::~PostHandler() {}

HttpResponse PostHandler::handle(const HttpRequest &req, const RouteConfig& route) const
{
    (void)route; // Suppress unused parameter warning
    HttpResponse resp;
    // TODO
    //*-> POST method logic implimented here;
    resp.version = req.version;
    resp.statusCode = 501; // Not Implemented
    resp.statusText = "Not Implemented";
    resp.headers["Allow"] = "POST"; // Indicating that POST is allowed
    resp.body = "POST method is not implemented yet.";
    resp.headers["Content-Length"] = std::to_string(resp.body.size());
    return resp;
}