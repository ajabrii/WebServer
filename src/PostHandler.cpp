/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PostHandler.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/01 18:26:13 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/12 10:51:42 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


// PostHandler.cpp
#include "../includes/PostHandler.hpp"
#include <fstream>
#include <sstream>
#include <ctime>
#include <map>
#include <vector>
#include <algorithm>
#include <iostream>

// ----- helpers -----

std::string extractBoundary(const std::string& ct)
{
    size_t pos = ct.find("boundary=");
    if (pos != std::string::npos)
        return ct.substr(pos + 9); // skip 'boundary='
    return "";
}

void writeFile(const std::string& path, const std::string& content) {
    std::ofstream out(path.c_str(), std::ios::binary);
    if (!out) throw std::runtime_error("Failed to write file: " + path);
    out << content;
}

void writeKeyValuesToFile(const std::string& path, const std::map<std::string,std::string>& fields)
{
    std::ofstream out(path.c_str());
    if (!out)
        throw std::runtime_error("Failed to write file: " + path);
    for (std::map<std::string,std::string>::const_iterator it=fields.begin(); it!=fields.end(); ++it)
        out << it->first << "=" << it->second << "\n";
}
//improve later on
std::vector<Part> parseMultipart(const std::string& body, const std::string& boundary)
{
    std::vector<Part> parts;
    std::string sep = "--" + boundary;  // i added -- because mulipart boundaries are prefixed with -- acording to RFC 2046 (for more data check https://datatracker.ietf.org/doc/html/rfc2046#section-5.1.1)
    std::cout << "line 123 :sep: " << sep << std::endl; // remove this later
    size_t pos = 0;

    while ((pos = body.find(sep, pos)) != std::string::npos)
    {
        pos += sep.size();
        if (body.substr(pos,2) == "--")
            break; // end
        if (body.substr(pos,2) == "\r\n")
            pos += 2;
        size_t headerEnd = body.find("\r\n\r\n", pos);
        if (headerEnd == std::string::npos)
            break;
        std::string header = body.substr(pos, headerEnd - pos);
        pos = headerEnd + 4;

        size_t nextSep = body.find(sep, pos);
        if (nextSep == std::string::npos)
            break;

        std::string content = body.substr(pos, nextSep - pos);
        if (!content.empty() && content[content.size()-2]=='\r' && content[content.size()-1]=='\n')
            content = content.substr(0, content.size()-2); // strip trailing \r\n

        Part part;
        size_t fnPos = header.find("filename=\"");
        if (fnPos != std::string::npos) {
            size_t fnEnd = header.find("\"", fnPos+10);
            part.filename = header.substr(fnPos+10, fnEnd - (fnPos+10));
        }
        size_t namePos = header.find("name=\"");
        if (namePos != std::string::npos) {
            size_t nameEnd = header.find("\"", namePos+6);
            part.name = header.substr(namePos+6, nameEnd - (namePos+6));
        }
        part.content = content;
        parts.push_back(part);
        pos = nextSep;
    }
    return parts;
}

std::map<std::string,std::string> parseFormUrlEncoded(const std::string& body)
{
    std::map<std::string,std::string> fields;
    std::istringstream ss(body); //
    std::string pair;
    while (std::getline(ss, pair, '&'))
    {
        size_t eq = pair.find('=');
        if (eq != std::string::npos)
        {
            std::string key = pair.substr(0, eq);
            std::string value = pair.substr(eq+1);
            fields[key] = value;
        }
    }
    return fields;
}

// Helper to build error response
HttpResponse makeErrorResponse(int code, const std::string& text) {
    HttpResponse resp;
    resp.statusCode = code;
    resp.statusText = text;
    resp.body = text;
    resp.headers["Content-Length"] = std::to_string(resp.body.size());
    return resp;
}

// ----- PostHandler methods -----
PostHandler::PostHandler() { }

PostHandler::~PostHandler() { }

HttpResponse PostHandler::handle(const HttpRequest &req, const RouteConfig& route) const
{
    HttpResponse resp;
    resp.version = req.version;

    std::string ct = req.GetHeader("content-type");
    // std::cout << "line 125 :Content-Type: " << ct << std::endl; // remove this later
    std::transform(ct.begin(), ct.end(), ct.begin(), ::tolower); // transform to lowercase because content-type is case-insensitive
    // std::cout << "route.uploadDir: " << route.uploadDir << std::endl; // remove this later
    if (route.uploadDir.empty()) {
        return makeErrorResponse(500, "Upload directory not configured."); //! Use error page from ServerConfig if available
    }
    try {
        if (ct.find("multipart/form-data") != std::string::npos) {
            std::string boundary = extractBoundary(ct);
            if (boundary.empty())
                return makeErrorResponse(400, "Missing boundary in multipart data."); //! Use error page from ServerConfig if available

            std::vector<Part> parts = parseMultipart(req.body, boundary);
            bool saved = false; // this is for the case where no file is found in the multipart data
            for (size_t i=0; i<parts.size(); ++i)
            {
                if (!parts[i].filename.empty())
                {
                    std::string filepath = route.uploadDir + "/" + parts[i].filename;
                    writeFile(filepath, parts[i].content);
                    // resp.headers["Content-Type"] = "text/html; charset=UTF-8";
                    resp.body = "File uploaded: " + filepath;
                    saved = true;
                    break;
                }
            }
            if (!saved)
                return makeErrorResponse(400, "No file found in multipart data."); // Use error page from ServerConfig if available
        }
        else if (ct.find("application/x-www-form-urlencoded") != std::string::npos)
        {
            std::map<std::string,std::string> fields = parseFormUrlEncoded(req.body);
            std::string filepath = route.uploadDir + "/form_" + std::to_string(std::time(0)) + ".txt";
            writeKeyValuesToFile(filepath, fields);
            resp.version = "HTTP/1.1";
            // set the response content type to html
            resp.body = "Form data saved to: " + filepath;
        }
        else if (ct.find("application/json") != std::string::npos)
        {
            std::string filepath = route.uploadDir + "/json_" + std::to_string(std::time(0)) + ".json";
            writeFile(filepath, req.body);
            resp.body = "JSON saved to: " + filepath;
        }
        else {
            std::string filepath = route.uploadDir + "/data_" + std::to_string(std::time(0)) + ".txt";
            writeFile(filepath, req.body);
            resp.body = "Data saved to: " + filepath;
            // return makeErrorResponse(415, "Unsupported Media Type: " + ct);
        }
    }
    catch (const std::exception& e) {
        return makeErrorResponse(500, e.what()); // Use error page from ServerConfig if available
    }

    resp.statusCode = 201;
    resp.statusText = "Created";
    resp.headers["Content-Type"] = "text/html; charset=UTF-8";
    resp.headers["Content-Length"] = std::to_string(resp.body.size());
    return resp;
}

/**
 * New PostHandler method with ServerConfig support
 * Delegates to legacy method for now (can be enhanced later for custom error pages)
 */
HttpResponse PostHandler::handle(const HttpRequest &req, const RouteConfig& route, const ServerConfig& serverConfig) const {
    // For now, just delegate to the legacy method
    // This can be enhanced later to use custom error pages from serverConfig
    (void)serverConfig; // Suppress unused parameter warning
    return handle(req, route);
}
