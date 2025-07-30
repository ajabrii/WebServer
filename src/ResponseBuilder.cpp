/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ResponseBuilder.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/23 15:00:00 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/30 15:37:56 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/ResponseBuilder.hpp"
#include "../includes/Utils.hpp"
#include <ctime>

HttpResponse ResponseBuilder::createErrorResponse(int statusCode, const std::string &statusText, const ServerConfig &serverConfig)
{
    HttpResponse response;
    response.version = "HTTP/1.1";
    response.statusCode = statusCode;
    response.statusText = statusText;
    response.headers["content-type"] = "text/html; charset=utf-8";
    response.body = Error::loadErrorPage(statusCode, serverConfig);
    finalizeResponse(response);
    return response;
}

HttpResponse ResponseBuilder::createSuccessResponse(int statusCode, const std::string &statusText,
                                                    const std::string &body, const std::string &contentType)
{
    HttpResponse response;
    response.version = "HTTP/1.1";
    response.statusCode = statusCode;
    response.statusText = statusText;
    response.headers["content-type"] = contentType;
    response.body = body;

    finalizeResponse(response);
    return response;
}

HttpResponse ResponseBuilder::createJsonResponse(int statusCode, const std::string &statusText, const std::string &jsonBody)
{
    return createSuccessResponse(statusCode, statusText, jsonBody, "application/json; charset=utf-8");
}

HttpResponse ResponseBuilder::createRedirectResponse(int statusCode, const std::string &statusText, const std::string &location)
{
    HttpResponse response;
    response.version = "HTTP/1.1";
    response.statusCode = statusCode;
    response.statusText = statusText;
    response.headers["location"] = location;
    response.headers["content-type"] = "text/html; charset=utf-8";
    std::ostringstream html;
    html << "<!DOCTYPE html>\n";
    html << "<html>\n";
    html << "<head>\n";
    html << "  <title>" << statusCode << " " << statusText << "</title>\n";
    html << "  <meta http-equiv=\"refresh\" content=\"0; url=" << location << "\">\n";
    html << "</head>\n";
    html << "<body>\n";
    html << "  <h1>" << statusCode << " " << statusText << "</h1>\n";
    html << "  <p>The document has moved <a href=\"" << location << "\">here</a>.</p>\n";
    html << "  <p>You will be redirected automatically.</p>\n";
    html << "</body>\n";
    html << "</html>";
    response.body = html.str();
    finalizeResponse(response);
    return response;
}

HttpResponse ResponseBuilder::createDeleteSuccessResponse(const std::string &filePath)
{
    std::ostringstream json;
    json << "{\n";
    json << "  \"success\": true,\n";
    json << "  \"message\": \"File deleted successfully\",\n";
    json << "  \"file\": \"" << filePath << "\",\n";
    json << "  \"timestamp\": \"" <<"13:37:42" << "\"\n";
    json << "}";

    return createJsonResponse(200, "OK", json.str());
}

void ResponseBuilder::finalizeResponse(HttpResponse &response)
{
    if (response.version.empty())
        response.version = "HTTP/1.1";
    response.headers["content-length"] = Utils::toString(response.body.size());
    response.headers["server"] = "Webserv/1.0";
}
