/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: baouragh <baouragh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/14 12:00:00 by ajabri            #+#    #+#             */
/*   Updated: 2025/08/05 13:34:06 by baouragh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Utils.hpp"
#include "../includes/ResponseBuilder.hpp"
#include "../includes/SessionManager.hpp"
#include "../includes/SessionID.hpp"
#include "../includes/CookieParser.hpp"
#include "../includes/Connection.hpp"
#include "../includes/HttpRequest.hpp"
#include <string>
#include <map>
#include <algorithm>


std::string clean_line(std::string line)
{
    line.erase(0, line.find_first_not_of(" \t\n\r"));
    line.erase(line.find_last_not_of(" \t\n\r") + 1);

    if (!line.empty() && line[line.size() - 1] == ';')
        line.erase(line.size() - 1);
    return line;
}

bool shouldKeepAlive(const HttpRequest &request)
{
    std::string connection = request.GetHeader("connection");
    std::transform(connection.begin(), connection.end(), connection.begin(), ::tolower);

    if (connection == "close")
        return false;
    return true;
}

void setConnectionHeaders(HttpResponse &response, bool keepAlive)
{
    if (keepAlive)
    {
        response.headers["Connection"] = "keep-alive";
        response.headers["Keep-Alive"] = "timeout=60, max=100";
    }
    else
        response.headers["Connection"] = "close";
}

std::string toLower(const std::string& s)
{
    std::string result = s;
    for (size_t i = 0; i < result.size(); ++i)
        result[i] = std::tolower(result[i]);
    return result;
}

void ltrim(std::string& s)
{
    size_t i = 0;
    while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) ++i;
    s.erase(0, i);
}

HttpResponse parseCgiOutput(const std::string& raw, const ServerConfig& serverConfig) 
{
    for (size_t i = 0; i < raw.size(); ++i) {
    if (raw[i] == '\r') std::cerr << "\\r";
    else if (raw[i] == '\n') std::cerr << "\\n";
    else std::cerr << raw[i];
}
std::cerr << std::endl;

    HttpResponse response;


    size_t headerEnd = raw.find("\r\n\r\n");
    if (headerEnd == std::string::npos) 
    {
        std::cerr << "[CGI] Invalid CGI output: no headers found." << std::endl;
        response.statusCode = 500;
        response.statusText = "Internal Server Error";
        response.body = Error::loadErrorPage(500, serverConfig);
        response.headers["Content-Type"] = "text/html";
        response.headers["Content-Length"] = Utils::toString(response.body.size());
        return response;
    }
    std::string headerPart = raw.substr(0, headerEnd);
    std::string bodyPart = raw.substr(headerEnd + 4);
    std::istringstream headerStream(headerPart);
    std::string line;
    bool statusParsed = false;
    if (std::getline(headerStream, line))
    {
        if (!line.empty() && *line.rbegin() == '\r')
            line.erase(line.size() - 1);
        if (line.compare(0, 5, "HTTP/") == 0) 
        {
            std::istringstream statusStream(line);
            std::string httpVersion;
            statusStream >> httpVersion >> response.statusCode;
            std::getline(statusStream, response.statusText);
            ltrim(response.statusText);
            statusParsed = true;
        } 
        else 
        {
            headerStream.clear();
            headerStream.str(headerPart);
        }
    }
    while (std::getline(headerStream, line)) 
    {
        if (!line.empty() && *line.rbegin() == '\r')
            line.erase(line.size() - 1);
        size_t colon = line.find(':');
        if (colon != std::string::npos) 
        {
            std::string key = line.substr(0, colon);
            std::string value = line.substr(colon + 1);
            ltrim(value);
            std::string lowerKey = toLower(key);
            if (lowerKey == "status") 
            {
                std::istringstream statusStream(value);
                statusStream >> response.statusCode;
                std::getline(statusStream, response.statusText);
                ltrim(response.statusText);
                statusParsed = true;
            }
            if (lowerKey == "set-cookie")
                response.CookiesHeaders.insert(std::make_pair("Set-Cookie", value));
            else
                response.headers[lowerKey] = value;
        }
    }
    if (!statusParsed) 
    {
        response.statusCode = 200;
        response.statusText = "OK";
    }
    response.body = bodyPart;
    if (response.headers.find("content-length") == response.headers.end()) 
    {
        std::ostringstream oss;
        oss << response.body.size();
        response.headers["Content-Length"] = oss.str();
    }
    if (response.headers.find("content-type") == response.headers.end()) 
    {
        response.statusCode = 502;
        response.statusText = "Bad Gateway";
        response.body = Error::loadErrorPage(502, serverConfig);
        response.headers["Content-Type"] = "text/html";
        response.headers["Content-Length"] = Utils::toString(response.body.size());
        std::cerr << "[CGI] Warning: Content-Type not found in headers, set to text/html" << std::endl;
    }
    return response;
}

void HandleCookies(Connection& conn, HttpRequest& req)
{
    SessionManager sessionManager;
    SessionInfos& sessionInfos = conn.getSessionInfos();
        
    if (req.headers.find("cookie") != req.headers.end()) 
    {
        std::string cookieHeader = req.headers["cookie"];
        std::map<std::string, std::string> cookies = CookieParser::parse(cookieHeader);

        sessionInfos.setCookies(cookies);

        if (cookies.find("session_id") != cookies.end())
        {
            std::string sessionId = cookies["session_id"];
            sessionInfos.setSessionId(sessionId);
            if (access(sessionManager.buildSessionFilePath(sessionId).c_str(), F_OK) != 0
            || access(sessionManager.buildSessionFilePath(sessionId).c_str(), R_OK) != 0
            || access(sessionManager.buildSessionFilePath(sessionId).c_str(), W_OK) != 0)
            {
                sessionManager.save(sessionId, sessionInfos.getCookies());
            }
        }
        else
        {
            std::string newSessionId = SessionID::generate(&conn, conn.getRequestCount());
            sessionInfos.setSessionId(newSessionId);
            sessionInfos.getCookies()["session_id"] = newSessionId;

            if (access(sessionManager.buildSessionFilePath(newSessionId).c_str(), F_OK) != 0
            || access(sessionManager.buildSessionFilePath(newSessionId).c_str(), R_OK) != 0
            || access(sessionManager.buildSessionFilePath(newSessionId).c_str(), W_OK) != 0)
            {
                sessionManager.save(newSessionId, sessionInfos.getCookies());
            }
        }
    }
    else 
    {
        std::cout << "\033[1;33m[Session]\033[0m No cookies found in request" << std::endl;
        std::string newSessionId = SessionID::generate(&conn, conn.getRequestCount());
        sessionInfos.setSessionId(newSessionId);
        sessionInfos.getCookies()["session_id"] = newSessionId;
        if (access(sessionManager.buildSessionFilePath(newSessionId).c_str(), F_OK) != 0
        || access(sessionManager.buildSessionFilePath(newSessionId).c_str(), R_OK) != 0
        || access(sessionManager.buildSessionFilePath(newSessionId).c_str(), W_OK) != 0)
        {
            sessionManager.save(newSessionId, sessionInfos.getCookies());
        }
    }
    sessionInfos.setSessionPath();
    sessionInfos.setSessionId(sessionInfos.getSessionId());
    conn.getCurrentRequest().SessionId = conn.getSessionInfos().getSessionId();
    // sessionInfos.setSessionData(sessionManager.load(sessionInfos.getSessionId()));
        
}

void handleNewConnection(Reactor &reactor, const Event &event)
{
    HttpServer* server = reactor.getServerByListeningFd(event.fd);
    std::cout << "\033[1;32m[+]\033[0m New connection detected on fd: " << event.fd << std::endl;
    if (server)
    {
        Connection* conn = new Connection(server->acceptConnection(event.fd));
        conn->updateLastActivity();
        reactor.addConnection(conn, server);
        std::cout << NEW_CLIENT_CON << std::endl;
        bool keepAlive = shouldKeepAlive(conn->getCurrentRequest());      
        if (conn->getRequestCount() >= REQUEST_LIMIT_PER_CONNECTION)
            keepAlive = false;
        conn->setKeepAlive(keepAlive);
    }
}

HttpResponse createErrorResponse(int statusCode, const std::string &statusText, const ServerConfig &ServerConfig)
{
    return ResponseBuilder::createErrorResponse(statusCode, statusText, ServerConfig);
}

void processReadableEvent(Reactor &reactor, const Event &event, const std::string &cgiEnv)
{
    Connection &conn = reactor.getConnection(event.fd);
    CgiState *cgiState = conn.getCgiState();
    ServerConfig serverConfig = reactor.getServerForClient(conn.getFd())->getConfig();

    if (cgiState)
    {
        cgiState->writeToScript(conn);
        cgiState->readFromScript(conn, reactor, serverConfig);
    }
    else
    {
        HttpServer *server = reactor.getServerForClient(event.fd);
        if (!server)
        {
            reactor.removeConnection(event.fd);
            std::cerr << "Error: No server found for client fd: " << event.fd << std::endl;
        }
        try {
            conn.readData(server);
        } catch (const HttpRequest::HttpException &e) 
        {
            std::cerr << "Connection read error: " << e.what() << std::endl;
            HttpResponse errorResp = createErrorResponse(e.getStatusCode(), e.what(), server->getConfig());
            conn.writeData(errorResp.toString());
            reactor.removeConnection(event.fd);
            return;
        }
        if (conn.isRequestComplete())
        {
            processHttpRequest(reactor, conn, server, event, cgiEnv);
        }
    }
}

void processHttpRequest(Reactor &reactor, Connection &conn, HttpServer *server, const Event &event, const std::string &cgiEnv)
{
    HttpRequest &req = conn.getCurrentRequest();
    try {
        HandleCookies(conn, req);
        Router router;
        const RouteConfig *route = router.match(req, server->getConfig());
        HttpResponse resp;
        if (route)
        {
            CgiHandler cgi(*server, req, *route, event.fd, cgiEnv);

            if (cgi.IsCgi())
            {
                try
                {
                    conn.setCgiState(cgi.execCgi(conn));
                    std::cout << "\033[1;34m[CGI]\033[0m Executing CGI script: " << std::endl;
                    reactor.watchCgi(&conn);
                    return ;
                }   
                catch (const HttpRequest::HttpException &e)
                {
                    std::cerr << "[CGI ERROR] read error: " << e.what() << std::endl;
                    HttpResponse errorResp = createErrorResponse(e.getStatusCode(), e.what(), server->getConfig());
                    conn.writeData(errorResp.toString());
                    reactor.removeConnection(event.fd);
                    return ;
                }
            }
            else
            {
                RequestDispatcher dispatcher;
                resp = dispatcher.dispatch(req, *route, server->getConfig());
            }
        }
        else
        {
            resp = createErrorResponse(404, "Not Found", server->getConfig());
            conn.writeData(resp.toString());
            reactor.removeConnection(event.fd);
        }

    handleHttpResponse(reactor, conn, resp, req);
    } catch (const std::runtime_error &e) {
        handleHttpException(reactor, conn, server, e);
    }
}

void handleHttpResponse(Reactor &reactor, Connection &conn, HttpResponse &resp, const HttpRequest &req)
{
    bool keepAlive = shouldKeepAlive(req);
    if (conn.getRequestCount() >= REQUEST_LIMIT_PER_CONNECTION)
    {
        keepAlive = false;
    }

    setConnectionHeaders(resp, keepAlive);
    resp.SetCookieHeaders(conn);
    conn.writeData(resp.toString());
    conn.reset();
    conn.updateLastActivity();
    if (keepAlive)
    {
        conn.setKeepAlive(true);
        conn.incrementRequestCount();
        conn.resetForNextRequest();
        std::cout << "\033[1;32m[+]\033[0m Connection kept alive (request #"
                  << conn.getRequestCount() << ")" << std::endl;
    }
    else
    {
        reactor.removeConnection(conn.getFd());
        std::cout << "\033[1;31m[-]\033[0m Connection closed" << std::endl;
    }
}

void handleHttpException(Reactor &reactor, Connection &conn, HttpServer *server, const std::runtime_error &e)
{
    std::string msg = e.what();

    if (msg.find("incomplete body") != std::string::npos)
    {
        return;
    }
    HttpResponse errorResp = createErrorResponse(400, "Bad Request", server->getConfig());
    setConnectionHeaders(errorResp, false);
    errorResp.SetCookieHeaders(conn);
    conn.writeData(errorResp.toString());
    reactor.removeConnection(conn.getFd());
}