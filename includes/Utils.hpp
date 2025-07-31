/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/14 12:00:00 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/31 17:18:54 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
#define UTILS_HPP

#include "../includes/HttpServer.hpp"
#include "../includes/Reactor.hpp"
#include "../includes/Router.hpp"
#include "../includes/RequestDispatcher.hpp"
#include "../includes/CgiHandler.hpp"
#include "../includes/Errors.hpp"
#include "../includes/Utils.hpp"
#include <sstream>
#include <string>

#define REQUEST_LIMIT_PER_CONNECTION 100

namespace Utils
{
    template<typename T>
    std::string toString(const T& value) {
        std::stringstream ss;
        ss << value;
        return ss.str();
    }
}

bool shouldKeepAlive(const HttpRequest& request);
void setConnectionHeaders(HttpResponse& response, bool keepAlive);
std::string toLower(const std::string& s);
void ltrim(std::string& s);
HttpResponse parseCgiOutput(const std::string& raw);
std::string buildSetCookieHeader(const std::string& session_id);
void HandleCookies(Connection& conn, HttpRequest& req);
void handleNewConnection(Reactor &reactor, const Event &event);
void processReadableEvent(Reactor &reactor, const Event &event, const std::string &cgiEnv);
HttpResponse createErrorResponse(int statusCode, const std::string &statusText, const ServerConfig &ServerConfig);
void processHttpRequest(Reactor &reactor, Connection &conn, HttpServer *server, const Event &event, const std::string &cgiEnv);
void handleHttpResponse(Reactor &reactor, Connection &conn, HttpResponse &resp, const HttpRequest &req);
void handleHttpException(Reactor &reactor, Connection &conn, HttpServer *server, const std::runtime_error &e);



#endif