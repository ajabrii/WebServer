/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/14 12:00:00 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/23 11:25:27 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Utils.hpp"

std::string clean_line(std::string line)
{
    line.erase(0, line.find_first_not_of(" \t\n\r"));
    line.erase(line.find_last_not_of(" \t\n\r") + 1);

    if (!line.empty() && line[line.size() - 1] == ';')
    {
        line.erase(line.size() - 1);
    }
    return line;
}

bool shouldKeepAlive(const HttpRequest& request)
{
    std::string connection = request.GetHeader("connection");
    std::transform(connection.begin(), connection.end(), connection.begin(), ::tolower); // http is case-insensitive

    if (connection == "close")
        return false;
    return true;
}

void setConnectionHeaders(HttpResponse& response, bool keepAlive)
{
    if (keepAlive) {
        response.headers["Connection"] = "keep-alive";
        response.headers["Keep-Alive"] = "timeout=60, max=100";
    } else {
        response.headers["Connection"] = "close";
    }
}




// ========================= UTILITY FUNCTIONS =========================
std::string toLower(const std::string &s)
{
    std::string result = s;
    for (size_t i = 0; i < result.size(); ++i)
    {
        result[i] = std::tolower(result[i]);
    }
    return result;
}

void ltrim(std::string &s)
{
    size_t i = 0;
    while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i])))
    {
        ++i;
    }
    s.erase(0, i);
}

// ========================= CGI OUTPUT PARSER =========================
HttpResponse parseCgiOutput(const std::string &raw)
{
    HttpResponse response;

    size_t headerEnd = raw.find("\r\n\r\n");
    if (headerEnd == std::string::npos)
    {
        response.statusCode = 500;
        response.statusText = "Internal Server Error";
        response.body = "CGI script did not return valid headers.";
        return response;
    }

    std::string headerPart = raw.substr(0, headerEnd);
    std::string bodyPart = raw.substr(headerEnd + 4);

    std::istringstream headerStream(headerPart);
    std::string line;
    bool statusParsed = false;

    while (std::getline(headerStream, line))
    {
        if (!line.empty() && *line.rbegin() == '\r')
        {
            line.erase(line.size() - 1);
        }

        size_t colon = line.find(':');
        if (colon != std::string::npos)
        {
            std::string key = line.substr(0, colon);
            std::string value = line.substr(colon + 1);
            ltrim(value);
            key = toLower(key);

            if (key == "status")
            {
                std::istringstream statusStream(value);
                statusStream >> response.statusCode;
                std::getline(statusStream, response.statusText);
                ltrim(response.statusText);
                statusParsed = true;
            }
            else
            {
                response.headers[key] = value;
            }
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
        response.headers["content-length"] = oss.str();
    }

    return response;
}

void handleNewConnection(Reactor &reactor, const Event &event)
{
    HttpServer *server = reactor.getServerByListeningFd(event.fd);
    if (server)
    {
        Connection *conn = new Connection(server->acceptConnection(event.fd));
        conn->updateLastActivity();
        reactor.addConnection(conn, server);
        std::cout << NEW_CLIENT_CON << std::endl;
    }
}


// ========================= HELPER FUNCTIONS =========================

void processReadableEvent(Reactor &reactor, const Event &event, const std::string &cgiEnv)
{
    Connection &conn = reactor.getConnection(event.fd);
    CgiState *cgiState = conn.getCgiState();

    // Handle CGI process communication
    if (cgiState)
    {
        handleCgiState(reactor, conn, cgiState, event);
        return;
    }

    // Handle regular HTTP request
    HttpServer *server = reactor.getServerForClient(event.fd);
    if (!server)
    {
        reactor.removeConnection(event.fd);
        std::cerr << "Error: No server found for client fd: " << event.fd << std::endl;
        return;
    }

    // Read data from client
    try
    {
        conn.readData(server);
    }
    catch (const HttpRequest::HttpException &e)
    {
        std::cerr << "Connection read error: " << e.what() << std::endl;
        HttpResponse errorResp = createErrorResponse(e.getStatusCode(), e.what(), server->getConfig());
        conn.writeData(errorResp.toString());
        reactor.removeConnection(event.fd);
        return;
    }

    // Process complete requests
    if (conn.isRequestComplete())
    {
        processHttpRequest(reactor, conn, server, event, cgiEnv);
    }
}

void processHttpRequest(Reactor &reactor, Connection &conn, HttpServer *server,
                        const Event &event, const std::string &cgiEnv)
{
    HttpRequest &req = conn.getCurrentRequest();

    try
    {
        // Route matching
        Router router;
        const RouteConfig *route = router.match(req, server->getConfig());
        HttpResponse resp;

        if (route)
        {
            // Check if request should be handled by CGI
            CgiHandler cgi(*server, req, *route, event.fd, cgiEnv);

            if (cgi.IsCgi())
            {
                handleCgiRequest(reactor, conn, cgi, server->getConfig());
                return;
            }
            else
            {
                // Handle regular HTTP request
                RequestDispatcher dispatcher;
                resp = dispatcher.dispatch(req, *route, server->getConfig());
            }
        }
        else
        {
            resp = createErrorResponse(404, "Not Found", server->getConfig());
        }

        // Send response and handle connection persistence
        handleHttpResponse(reactor, conn, resp, req);
    }
    catch (const std::runtime_error &e)
    {
        handleHttpException(reactor, conn, server, e);
    }
    // catch (const std::exception &e)
    // {
    //     Error::logs("Connection error: " + std::string(e.what()));
    //     reactor.removeConnection(event.fd);
    // }
}

void handleCgiRequest(Reactor &reactor, Connection &conn, CgiHandler &cgi, const ServerConfig &ServerConfig)
{
    conn.setCgiState(cgi.execCgi());

    if (conn.getCgiState())
    {
        reactor.watchCgi(&conn);
    }
    else
    {
        Error::logs("CGI execution failed");
        HttpResponse resp = createErrorResponse(500, "Internal Server Error", ServerConfig);
        conn.writeData(resp.toString());
        reactor.removeConnection(conn.getFd());
    }
}

void handleHttpResponse(Reactor &reactor, Connection &conn, HttpResponse &resp,
                        const HttpRequest &req)
{
    // Determine if connection should be kept alive
    bool keepAlive = shouldKeepAlive(req);

    if (conn.getRequestCount() >= REQUEST_LIMIT_PER_CONNECTION)
    {
        keepAlive = false;
    }

    // Set connection headers and send response
    setConnectionHeaders(resp, keepAlive);
    conn.writeData(resp.toString());
    conn.reset();
    conn.updateLastActivity();

    // Handle connection persistence
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

void handleHttpException(Reactor &reactor, Connection &conn, HttpServer *server,
                         const std::runtime_error &e)
{
    std::string msg = e.what();

    if (msg.find("incomplete body") != std::string::npos)
    {
        return;
    }

    // Handle parsing errors
    std::cerr << "Parse error: " << msg << std::endl;

    HttpResponse errorResp;
    errorResp.version = "HTTP/1.1";
    errorResp.statusCode = 400;
    errorResp.statusText = "Bad Request";
    errorResp.headers["content-type"] = "text/html";
    errorResp.body = Error::loadErrorPage(400, server->getConfig());
    errorResp.headers["content-length"] = Utils::toString(errorResp.body.size());

    setConnectionHeaders(errorResp, false);
    conn.writeData(errorResp.toString());
    reactor.removeConnection(conn.getFd());
}

// ========================= RESPONSE BUILDERS =========================

HttpResponse createErrorResponse(int statusCode, const std::string &statusText, const ServerConfig &ServerConfig)
{
    HttpResponse resp;
    resp.version = "HTTP/1.1";
    resp.statusCode = statusCode;
    resp.statusText = statusText;
    resp.headers["content-type"] = "text/html";
    resp.body = Error::loadErrorPage(statusCode, ServerConfig);
    resp.headers["content-length"] = Utils::toString(resp.body.size());
    return resp;
}
