/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PostHandler.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/01 18:26:13 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/04 08:21:56 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


# include "../includes/PostHandler.hpp"
#include <fstream>
#include <ctime>

PostHandler::PostHandler(){ }

PostHandler::~PostHandler() {}

HttpResponse PostHandler::handle(const HttpRequest &req, const RouteConfig& route) const
{
    HttpResponse resp;
    resp.version = req.version;

    // Check if upload directory is configured
    // route.uploadDir = "uploads"; // Example upload directory, replace with actual config retrieval
    if (route.uploadDir.empty()) {
        resp.statusCode = 500;
        resp.statusText = "Internal Server Error";
        resp.body = "Upload directory is not configured.";
        resp.headers["Content-Length"] = std::to_string(resp.body.size());
        return resp;
    }

    // Build file path (use timestamp or counter to avoid conflicts)
    std::string filePath = route.uploadDir + "/upload_" + std::to_string(std::time(0));

    std::ofstream file(filePath.c_str(), std::ios::binary);
    if (!file) {
        resp.statusCode = 500;
        resp.statusText = "Internal Server Error";
        resp.body = "Failed to open file for writing.";
        resp.headers["Content-Length"] = std::to_string(resp.body.size());
        return resp;
    }

    // Write request body to file
    file << req.body;
    file.close();

    resp.statusCode = 201; // Created
    resp.statusText = "Created";
    resp.body = "File uploaded successfully as " + filePath;
    resp.headers["Content-Length"] = std::to_string(resp.body.size());

    return resp;
}
