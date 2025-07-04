/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DeleteHandler.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/01 18:29:35 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/04 15:48:49 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "../includes/DeleteHandler.hpp"

DeleteHandler::DeleteHandler(){ }

DeleteHandler::~DeleteHandler() {}

HttpResponse DeleteHandler::handle(const HttpRequest &req, const RouteConfig& route) const
{
    HttpResponse resp;
    resp.version = req.version;

    // Compute full file path
    std::string filePath = route.root + req.uri;

    // Attempt to delete the file
    if (std::remove(filePath.c_str()) == 0) {
        // Success
        resp.statusCode = 200;
        resp.statusText = "OK";
        resp.body = "File deleted: " + filePath;
    } else {
        // Failed → check if file exists
        std::ifstream file(filePath.c_str());
        if (!file) {
            resp.statusCode = 404;
            resp.statusText = "Not Found";
            resp.body = "File not found: " + filePath;
        } else {
            resp.statusCode = 500;
            resp.statusText = "Internal Server Error";
            resp.body = "Could not delete file: " + filePath;
        }
    }

    resp.headers["Content-Length"] = std::to_string(resp.body.size());
    resp.headers["Content-Type"] = "text/plain";

    return resp;
}
