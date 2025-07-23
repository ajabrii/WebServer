/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ResponseBuilder.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/23 15:00:00 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/23 15:16:23 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "HttpResponse.hpp"
#include "ServerConfig.hpp"
#include "Utils.hpp"
#include "Errors.hpp"
#include <string>
#include <sstream>

class ResponseBuilder
{
public:
    static HttpResponse createErrorResponse(int statusCode, const std::string &statusText, const ServerConfig &serverConfig);
    static HttpResponse createSuccessResponse(int statusCode, const std::string &statusText,
                                              const std::string &body, const std::string &contentType = "text/html");
    static HttpResponse createJsonResponse(int statusCode, const std::string &statusText, const std::string &jsonBody);
    static HttpResponse createRedirectResponse(int statusCode, const std::string &statusText, const std::string &location);
    static HttpResponse createDeleteSuccessResponse(const std::string &filePath);

private:
    static void finalizeResponse(HttpResponse &response);
    static std::string getCurrentTimestamp();
};
