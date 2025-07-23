/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/14 12:00:00 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/23 15:35:55 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Utils.hpp"
#include "../includes/ResponseBuilder.hpp"

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

bool shouldKeepAlive(const HttpRequest &request)
{
    std::string connection = request.GetHeader("connection");
    std::transform(connection.begin(), connection.end(), connection.begin(), ::tolower); // http is case-insensitive

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
    {
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

    if (cgiState)
    {
        handleCgiState(reactor, conn, cgiState, event);
        return;
    }
    HttpServer *server = reactor.getServerForClient(event.fd);
    if (!server)
    {
        reactor.removeConnection(event.fd);
        std::cerr << "Error: No server found for client fd: " << event.fd << std::endl;
        return;
    }
    try {
        conn.readData(server);
    } catch (const HttpRequest::HttpException &e) {
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

void processHttpRequest(Reactor &reactor, Connection &conn, HttpServer *server, const Event &event, const std::string &cgiEnv)
{
    HttpRequest &req = conn.getCurrentRequest();
    try {
        Router router;
        const RouteConfig *route = router.match(req, server->getConfig());
        HttpResponse resp;

        if (route)
        {
            CgiHandler cgi(*server, req, *route, event.fd, cgiEnv);

            if (cgi.IsCgi())
            {
                handleCgiRequest(reactor, conn, cgi, server->getConfig());
                return;
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

void handleHttpResponse(Reactor &reactor, Connection &conn, HttpResponse &resp, const HttpRequest &req)
{
    bool keepAlive = shouldKeepAlive(req);
    if (conn.getRequestCount() >= REQUEST_LIMIT_PER_CONNECTION)
    {
        keepAlive = false;
    }

    setConnectionHeaders(resp, keepAlive);
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
    conn.writeData(errorResp.toString());
    reactor.removeConnection(conn.getFd());
}

// ========================= RESPONSE BUILDERS =========================

HttpResponse createErrorResponse(int statusCode, const std::string &statusText, const ServerConfig &ServerConfig)
{
    return ResponseBuilder::createErrorResponse(statusCode, statusText, ServerConfig);
}

void handleCgiState(Reactor &reactor, Connection &conn, CgiState *cgiState, const Event &event)
{
    if (!cgiState->bodySent && conn.getCurrentRequest().method == POST && cgiState->input_fd != -1)
    {
        ssize_t written = write(cgiState->input_fd,
                                conn.getCurrentRequest().body.c_str(),
                                conn.getCurrentRequest().body.size());
        if (written == -1)
        {
            perror("write to CGI stdin failed");
        }
        close(cgiState->input_fd);
        cgiState->input_fd = -1;
        cgiState->bodySent = true;
        std::cout << "\033[1;34m[CGI]\033[0m Body sent to CGI script for fd: "
                  << event.fd << std::endl;
    }

    char buffer[4096];
    ssize_t n = read(conn.getCgiState()->output_fd, buffer, sizeof(buffer));
    if (n > 0)
    {
        conn.getCgiState()->rawOutput.append(buffer, n);
    }
    else if (n == 0)
    {
        HttpResponse resp = parseCgiOutput(conn.getCgiState()->rawOutput);
        conn.writeData(resp.toString());
        reactor.removeConnection(event.fd);
        std::cout << "\033[1;31m[-]\033[0m Connection closed (CGI done)" << std::endl;
    }
    else
    {
        Error::logs("CGI read error on fd " + Utils::toString(event.fd));
        reactor.removeConnection(event.fd);
    }
}