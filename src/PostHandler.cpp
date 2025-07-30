/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PostHandler.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/01 18:26:13 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/30 11:30:21 by ajabri           ###   ########.fr       */
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
#include "../includes/Errors.hpp"


PostHandler::PostHandler() {}

PostHandler::~PostHandler() {}

HttpResponse PostHandler::handle(const HttpRequest &req, const RouteConfig &route, const ServerConfig &serverConfig) const
{
    std::cout << POST_LOG << " Handling POST request: uri=" << req.uri << std::endl;
    HttpResponse resp;
    resp.version = req.version;
    std::string originalCt = req.GetHeader("content-type");
    std::string ct = originalCt;
    std::transform(ct.begin(), ct.end(), ct.begin(), ::tolower);
    if (route.uploadDir.empty())
    {
        return makeErrorResponse(403, "Upload directory not configured.", serverConfig);
    }
    try
    {
        if (ct.find("multipart/form-data") != std::string::npos)
        {
            std::cout << POST_LOG << " Handling POST multipart/form-data: uri=" << req.uri << std::endl;
            std::string boundary = extractBoundary(originalCt);
            if (boundary.empty())
                return makeErrorResponse(400, "Missing boundary in multipart data.", serverConfig);

            std::vector<Part> parts = parseMultipart(req.body, boundary);

            std::vector<std::string> uploadedFiles;
            for (size_t i = 0; i < parts.size(); ++i)
            {
                if (!parts[i].filename.empty())
                {
                    std::string filepath = route.uploadDir + "/" + parts[i].filename;
                    writeFile(filepath, parts[i].content);
                    uploadedFiles.push_back(filepath);
                }
            }
            if (uploadedFiles.empty())
                return makeErrorResponse(400, "No file found in multipart data.", serverConfig);
            resp.body = "<html><head><title>Uploads</title></head><body><h1>Files uploaded:</h1><ul>";
            for (size_t i = 0; i < uploadedFiles.size(); ++i)
            {
                resp.body += "<li>" + uploadedFiles[i] + "</li>";
            }
            resp.body += "</ul></body></html>";
        }
        else if (ct.find("application/x-www-form-urlencoded") != std::string::npos)
        {
            std::cout << POST_LOG << " Handling POST application/x-www-form-urlencoded: uri=" << req.uri << std::endl;
            std::map<std::string, std::string> fields = parseFormUrlEncoded(req.body);
            std::string filepath = "./session/" + req.SessionId;
            writeKeyValuesToFile(filepath, fields);
            resp.version = "HTTP/1.1";
            resp.body = "<html><head><title>Uploads</title></head><body><h1>File uploaded: " + filepath + "</h1></body></html>";
        }
        else if (ct.find("application/json") != std::string::npos)
        {
            std::cout << POST_LOG << " Handling POST application/json: uri=" << req.uri << std::endl;
            std::string filepath = route.uploadDir + "/json_" + Utils::toString(std::time(0)) + ".json";
            writeFile(filepath, req.body);
            resp.body = "JSON saved to: " + filepath;
        }
        else
        {
            std::cout << POST_LOG << " Handling POST text/plain: uri=" << req.uri << std::endl;
            std::string filepath = route.uploadDir + "/data_" + Utils::toString(std::time(0)) + ".txt";
            writeFile(filepath, req.body);
            resp.body = "Data saved to: " + filepath;
        }
    }
    catch (const std::exception &e)
    {

        return makeErrorResponse(500, e.what(), serverConfig);
    }
    resp.statusCode = 201;
    resp.statusText = "Created";
    resp.headers["content-type"] = "text/html; charset=UTF-8";
    resp.headers["content-length"] = Utils::toString(resp.body.size());
    return resp;
}


std::string PostHandler::DataDecode(const std::string &encoded) const
{
    std::string decoded;

    for (size_t i = 0; i < encoded.length(); ++i)
    {
        if (encoded[i] == '%' && i + 2 < encoded.length())
        {
            std::string hexStr = encoded.substr(i + 1, 2);
            char *endPtr;
            long int value = std::strtol(hexStr.c_str(), &endPtr, 16);
            if (endPtr == hexStr.c_str() + 2 && value >= 0 && value <= 255)
            {
                decoded += static_cast<char>(value);
                i += 2;
            }
            else
            {
                decoded += encoded[i];
            }
        }
        else if (encoded[i] == '+')
        {
            decoded += ' ';
        }
        else
        {
            decoded += encoded[i];
        }
    }

    return decoded;
}

std::string PostHandler::extractBoundary(const std::string &ct) const
{
    size_t pos = ct.find("boundary=");
    if (pos != std::string::npos)
    {
        std::string boundary = ct.substr(pos + 9);
        size_t end = boundary.find(';');
        if (end != std::string::npos)
        {
            boundary = boundary.substr(0, end);
        }
        while (!boundary.empty() && (boundary[boundary.size() - 1] == ' ' ||
                                     boundary[boundary.size() - 1] == '\t' ||
                                     boundary[boundary.size() - 1] == '\r' ||
                                     boundary[boundary.size() - 1] == '\n'))
        {
            boundary = boundary.substr(0, boundary.size() - 1);
        }
        return boundary;
    }
    return "";
}

void PostHandler::writeFile(const std::string &path, const std::string &content) const
{
    std::ofstream out(path.c_str(), std::ios::binary);
    if (!out)
        throw std::runtime_error("Error: Failed to write file: " + path);
    out << content;
}

void PostHandler::writeKeyValuesToFile(const std::string &path, const std::map<std::string, std::string> &fields) const
{
    std::map<std::string, std::string> existingFields;
    std::ifstream in(path.c_str());
    if (in)
    {
        std::string line;
        while (std::getline(in, line))
        {
            size_t eq = line.find('=');
            if (eq != std::string::npos)
            {
                existingFields[line.substr(0, eq)] = line.substr(eq + 1);
            }
        }
    }
    for (std::map<std::string, std::string>::const_iterator it = existingFields.begin(); it != existingFields.end(); ++it)
    {
        std::cerr << "  " << it->first << ": " << it->second << std::endl;
    }
    for (std::map<std::string, std::string>::const_iterator it = fields.begin(); it != fields.end(); ++it)
    {
        existingFields[it->first] = it->second;
    }
    for (std::map<std::string, std::string>::const_iterator it = existingFields.begin(); it != existingFields.end(); ++it)
    {
        std::cerr << "  " << it->first << ": " << it->second << std::endl;
    }
    std::ofstream out(path.c_str(), std::ios::trunc);
    if (!out)
    {
        throw std::runtime_error("Error: Failed to write file: " + path);
    }
    for (std::map<std::string, std::string>::const_iterator it = existingFields.begin(); it != existingFields.end(); ++it)
    {
        out << DataDecode(it->first) << "=" << DataDecode(it->second) << "\n";
    }
}

std::vector<Part> PostHandler::parseMultipart(const std::string &body, const std::string &boundary) const
{
    std::vector<Part> parts;
    std::string sep = "--" + boundary;
    size_t pos = 0;
    
    pos = body.find(sep, pos);
    if (pos == std::string::npos)
    {
        return parts;
    }
    while (pos != std::string::npos)
    {
        pos += sep.size();
        if (pos + 2 <= body.size() && body.substr(pos, 2) == "--")
            break;
        if (pos + 2 <= body.size() && body.substr(pos, 2) == "\r\n")
            pos += 2;
        else if (pos + 1 <= body.size() && body.substr(pos, 1) == "\n")
            pos += 1;
        size_t headerEnd = body.find("\r\n\r\n", pos);
        size_t headerLength = 4;

        if (headerEnd == std::string::npos)
        {
            headerEnd = body.find("\n\n", pos);
            headerLength = 2;
            if (headerEnd == std::string::npos)
                break;
        }
        std::string header = body.substr(pos, headerEnd - pos);
        pos = headerEnd + headerLength;
        size_t nextSep = body.find(sep, pos);
        if (nextSep == std::string::npos)
        {
            nextSep = body.size();
        }
        std::string content = body.substr(pos, nextSep - pos);
        if (content.size() >= 2 && content.substr(content.size() - 2) == "\r\n")
            content = content.substr(0, content.size() - 2);
        else if (content.size() >= 1 && content.substr(content.size() - 1) == "\n")
            content = content.substr(0, content.size() - 1);
        Part part;
        size_t fnPos = header.find("filename=\"");
        if (fnPos != std::string::npos)
        {
            size_t fnEnd = header.find("\"", fnPos + 10);
            if (fnEnd != std::string::npos)
            {
                part.filename = header.substr(fnPos + 10, fnEnd - (fnPos + 10));
            }
        }
        size_t namePos = header.find("name=\"");
        if (namePos != std::string::npos)
        {
            size_t nameEnd = header.find("\"", namePos + 6);
            if (nameEnd != std::string::npos)
            {
                part.name = header.substr(namePos + 6, nameEnd - (namePos + 6));
            }
        }
        part.content = content;
        parts.push_back(part);
        pos = nextSep;
        if (pos >= body.size())
            break;
    }
    return parts;
}

std::map<std::string, std::string> PostHandler::parseFormUrlEncoded(const std::string &body) const
{
    std::map<std::string, std::string> fields;
    std::istringstream ss(body);
    std::string pair;
    while (std::getline(ss, pair, '&'))
    {
        size_t eq = pair.find('=');
        if (eq != std::string::npos)
        {
            std::string key = pair.substr(0, eq);
            std::string value = pair.substr(eq + 1);
            fields[key] = value;
        }
    }
    return fields;
}

HttpResponse PostHandler::makeErrorResponse(int code, const std::string &text, const ServerConfig &serverConfig) const
{
    HttpResponse resp;
    resp.statusCode = code;
    resp.statusText = text;
    resp.body = Error::loadErrorPage(code, serverConfig);
    resp.headers["content-length"] = Utils::toString(resp.body.size());
    return resp;
}
