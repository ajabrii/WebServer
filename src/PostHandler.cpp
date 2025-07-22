/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PostHandler.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/01 18:26:13 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/22 11:35:26 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/PostHandler.hpp"
#include "../includes/Utils.hpp"
#include <fstream>
#include <sstream>
#include <ctime>
#include <map>
#include <vector>
#include <algorithm>
#include <iostream>
# include "../includes/Errors.hpp"


std::string extractBoundary(const std::string& ct)
{
    size_t pos = ct.find("boundary=");
    if (pos != std::string::npos) {
        std::string boundary = ct.substr(pos + 9);
        // Remove any trailing semicolon or whitespace
        size_t end = boundary.find(';');
        if (end != std::string::npos) {
            boundary = boundary.substr(0, end);
        }
        // Trim whitespace from the end
        while (!boundary.empty() && (boundary[boundary.size()-1] == ' ' ||
                                   boundary[boundary.size()-1] == '\t' ||
                                   boundary[boundary.size()-1] == '\r' ||
                                   boundary[boundary.size()-1] == '\n')) {
            boundary = boundary.substr(0, boundary.size()-1);
        }
        return boundary;
    }
    return "";
}

void writeFile(const std::string& path, const std::string& content)
{
    std::ofstream out(path.c_str(), std::ios::binary);
    if (!out)
        throw std::runtime_error("Error: Failed to write file: " + path);
    out << content;
}

void writeKeyValuesToFile(const std::string& path, const std::map<std::string,std::string>& fields)
{
    std::cout <<"file path: `"<< path <<"'" <<std::endl;
    std::ofstream out(path.c_str());
    if (!out)
    {
        throw std::runtime_error("Error: Failed to write file: " + path);
    }
    for (std::map<std::string,std::string>::const_iterator it=fields.begin(); it!=fields.end(); ++it)
        out << it->first << "=" << it->second << "\n";
}

//improve later on
std::vector<Part> parseMultipart(const std::string& body, const std::string& boundary)
{
    std::vector<Part> parts;
    std::string sep = "--" + boundary;  // boundaries are prefixed with -- according to RFC 2046

    size_t pos = 0;

    // Find the first boundary
    pos = body.find(sep, pos);
    if (pos == std::string::npos) {
        return parts; // No boundary found
    }

    while (pos != std::string::npos)
    {
        pos += sep.size();

        // Check for end boundary (boundary followed by --)
        if (pos + 2 <= body.size() && body.substr(pos, 2) == "--") {
            break; // End of multipart data
        }

        // Skip line ending after boundary
        if (pos + 2 <= body.size() && body.substr(pos, 2) == "\r\n") {
            pos += 2;
        } else if (pos + 1 <= body.size() && body.substr(pos, 1) == "\n") {
            pos += 1;
        }

        // Find where headers end (double line break)
        size_t headerEnd = body.find("\r\n\r\n", pos);
        size_t headerLength = 4; // length of "\r\n\r\n"

        if (headerEnd == std::string::npos) {
            headerEnd = body.find("\n\n", pos);
            headerLength = 2; // length of "\n\n"
            if (headerEnd == std::string::npos) {
                break; // Invalid format
            }
        }

        // Extract headers
        std::string header = body.substr(pos, headerEnd - pos);

        // Move past the header end
        pos = headerEnd + headerLength;

        // Find next boundary
        size_t nextSep = body.find(sep, pos);
        if (nextSep == std::string::npos) {
            nextSep = body.size(); // Use end of body if no next boundary
        }

        // STEP 2: Handle . and .. path components
        // Extract content between current position and next boundary
        std::string content = body.substr(pos, nextSep - pos);

        // Remove trailing line ending from content (usually there's a \r\n before the next boundary)
        if (content.size() >= 2 && content.substr(content.size()-2) == "\r\n") {
            content = content.substr(0, content.size()-2);
        } else if (content.size() >= 1 && content.substr(content.size()-1) == "\n") {
            content = content.substr(0, content.size()-1);
        }

        Part part;

        // Parse filename from header
        size_t fnPos = header.find("filename=\"");
        if (fnPos != std::string::npos) {
            size_t fnEnd = header.find("\"", fnPos+10);
            if (fnEnd != std::string::npos) {
                part.filename = header.substr(fnPos+10, fnEnd - (fnPos+10));
            }
        }

        // Parse name from header
        size_t namePos = header.find("name=\"");
        if (namePos != std::string::npos) {
            size_t nameEnd = header.find("\"", namePos+6);
            if (nameEnd != std::string::npos) {
                part.name = header.substr(namePos+6, nameEnd - (namePos+6));
            }
        }

        part.content = content;
        parts.push_back(part);

        // Move to the next boundary
        pos = nextSep;
        if (pos >= body.size()) break;
    }
    return parts;
}

std::map<std::string,std::string> parseFormUrlEncoded(const std::string& body)
{
    std::map<std::string,std::string> fields;
    std::istringstream ss(body);
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

HttpResponse makeErrorResponse(int code, const std::string& text, const ServerConfig& serverConfig)
{
    HttpResponse resp;
    resp.statusCode = code;
    resp.statusText = text;
    resp.body = Error::loadErrorPage(code, serverConfig);
    resp.headers["content-length"] = Utils::toString(resp.body.size());
    return resp;
}

PostHandler::PostHandler() { }

PostHandler::~PostHandler() { }

HttpResponse PostHandler::handle(const HttpRequest &req, const RouteConfig& route, const ServerConfig& serverConfig) const
{
    HttpResponse resp;
    resp.version = req.version;

    std::string originalCt = req.GetHeader("content-type"); // Keep original case for boundary extraction
    std::string ct = originalCt;
    std::transform(ct.begin(), ct.end(), ct.begin(), ::tolower); // transform to lowercase because content-type is case-insensitive in http
    if (route.uploadDir.empty()) {
        return makeErrorResponse(500, "Upload directory not configured.", serverConfig);
    }
    try
    {
        if (ct.find("multipart/form-data") != std::string::npos)
        {
            // Extract boundary from ORIGINAL header (case sensitive!)
            std::string boundary = extractBoundary(originalCt);
            if (boundary.empty())
                return makeErrorResponse(400, "Missing boundary in multipart data.", serverConfig);

            std::vector<Part> parts = parseMultipart(req.body, boundary);

            bool saved = false;
            for (size_t i = 0; i < parts.size(); ++i)
            {
                if (!parts[i].filename.empty())
                {
                    std::string filepath = route.uploadDir + "/" + parts[i].filename;
                    writeFile(filepath, parts[i].content);
                    resp.body = "<html><head><title>Uploads</title></head><body><h1>File uploaded: " + filepath +"</h1></body></html>";
                    saved = true;
                    break;
                }
            }
            if (!saved)
                return makeErrorResponse(400, "No file found in multipart data.", serverConfig);
        }
        else if (ct.find("application/x-www-form-urlencoded") != std::string::npos)
        {
            std::map<std::string, std::string> fields = parseFormUrlEncoded(req.body);
            std::string filepath = route.uploadDir + "/form_" + Utils::toString(std::time(0)) + ".txt";
            writeKeyValuesToFile(filepath, fields);
            resp.version = "HTTP/1.1";
            resp.body = "<html><head><title>Uploads</title></head><body><h1>File uploaded: " + filepath +"</h1></body></html>";
        }
        else if (ct.find("application/json") != std::string::npos)
        {
            std::string filepath = route.uploadDir + "/json_" + Utils::toString(std::time(0)) + ".json";
            writeFile(filepath, req.body);
            resp.body = "JSON saved to: " + filepath;
        }
        else
        {
            std::string filepath = route.uploadDir + "/data_" + Utils::toString(std::time(0)) + ".txt";
            writeFile(filepath, req.body);
            resp.body = "Data saved to: " + filepath;
        }
    }
    catch (const std::exception& e) {

        return makeErrorResponse(500, e.what(), serverConfig);
    }
    resp.statusCode = 201;
    resp.statusText = "Created";
    resp.headers["content-type"] = "text/html; charset=UTF-8";
    resp.headers["content-length"] = Utils::toString(resp.body.size());
    return resp;
}

