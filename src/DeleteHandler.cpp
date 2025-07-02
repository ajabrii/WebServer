/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DeleteHandler.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/01 18:29:35 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/02 11:36:27 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "../includes/DeleteHandler.hpp"

DeleteHandler::DeleteHandler(){ }

DeleteHandler::~DeleteHandler() {}

HttpResponse DeleteHandler::handle(const HttpRequest &req, const RouteConfig& route) const
{
    (void)route; // Suppress unused parameter warning
    HttpResponse resp;
    // TODO
    //*-> DELETE method logic implimented here;
    resp.version = req.version;
    resp.statusCode = 501; // Not Implemented
    resp.statusText = "Not Implemented";
    resp.headers["Allow"] = "DELETE"; // Indicating that DELETE is allowed
    resp.body = "DELETE method is not implemented yet.";
    resp.headers["Content-Length"] = std::to_string(resp.body.size());
    return resp;
}