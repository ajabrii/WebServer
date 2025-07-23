/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/23 10:33:25 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/23 11:53:59 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Webserv.hpp"
#include <sstream>
#include <algorithm>
#include <cctype>
#include <signal.h>
#include <cstdlib>

// Static member definition - MUST HAVE THIS
Webserv *Webserv::instance = nullptr;

#define REQUEST_LIMIT_PER_CONNECTION 100

Webserv::Webserv(const std::string &configFilePath, char **envp) : shutdown(false)
{
    instance = this;
    initialize(configFilePath, envp);
}

Webserv::~Webserv()
{
    cleanup();
    instance = 0;
}

// ========================= STATIC SIGNAL HANDLER =========================

void Webserv::signalHandler(int signum)
{
    (void)signum;
    if (instance)
    {
        std::cout << "\n[INFO] Received shutdown signal..." << std::endl;
        instance->signalShutdown();
    }
}

// ========================= INITIALIZATION =========================

void Webserv::initialize(const std::string &configFilePath, char **envp)
{
    try
    {
        // Parse configuration
        ConfigInterpreter parser;
        parser.getConfigData(configFilePath);
        parser.parse();
        parser.checkValues();
        cgiEnv = parser.getPathForCGI(envp);
        std::cout << "[✔] Config loaded" << std::endl;

        // Setup servers
        std::vector<ServerConfig> configs = parser.getServerConfigs();
        setupServers(configs);

        // Setup signal handlers
        setupSignalHandlers();

        std::cout << "[✔] Webserv initialized" << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Initialization error: " << e.what() << std::endl;
        throw;
    }
}

void Webserv::setupServers(const std::vector<ServerConfig> &configs)
{
    for (size_t i = 0; i < configs.size(); ++i)
    {
        HttpServer *server = new HttpServer(configs[i]);
        server->setup();
        servers.push_back(server);
        reactor.registerServer(*server);
    }
}

void Webserv::setupSignalHandlers()
{
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
}

// ========================= MAIN INTERFACE =========================

void Webserv::run()
{
    std::cout << "[✔] Webserv starting..." << std::endl;
    eventLoop();
}

void Webserv::eventLoop()
{
    while (!shutdown)
    {
        try
        {
            // Poll for events
            reactor.poll();

            // Process all ready events BEFORE cleanup
            std::vector<Event> events = reactor.getReadyEvents();

            for (size_t i = 0; i < events.size(); ++i)
            {
                processEvent(events[i]);
            }

            // Cleanup timed out connections AFTER processing events
            reactor.cleanupTimedOutConnections();
        }
        catch (const std::exception &e)
        {
            std::string msg = e.what();

            // Handle poll interruption during shutdown gracefully
            if (msg.find("poll failed") != std::string::npos && shutdown)
            {
                std::cout << "[INFO] Poll interrupted by shutdown signal" << std::endl;
                break;
            }

            std::cerr << "Event loop error: " << e.what() << std::endl;
        }
    }
}

void Webserv::cleanup()
{
    std::cout << "\n[INFO] Shutting down gracefully..." << std::endl;

    // Cleanup reactor
    reactor.cleanup();

    // Cleanup servers
    for (size_t i = 0; i < servers.size(); ++i)
    {
        delete servers[i];
    }
    servers.clear();
}

// ========================= EVENT PROCESSING =========================

void Webserv::processEvent(const Event &event)
{
    if (event.isNewConnection)
    {
        handleNewConnection(event);
    }
    else if (event.isReadable || event.isPullHUP)
    {
        handleReadableEvent(event);
    }
    else if (event.isError)
    {
        handleErrorEvent(event);
    }
}

void Webserv::handleNewConnection(const Event &event)
{
    HttpServer *server = reactor.getServerByListeningFd(event.fd);
    if (server)
    {
        Connection *conn = new Connection(server->acceptConnection(event.fd));
        conn->updateLastActivity();
        reactor.addConnection(conn, server);
        std::cout << "[+] New client connection" << std::endl;
    }
}

void Webserv::handleReadableEvent(const Event &event)
{
    try
    {
        Connection &conn = reactor.getConnection(event.fd);
        CgiState *cgiState = conn.getCgiState();

        // Handle CGI process communication
        if (cgiState)
        {
            handleCgiState(conn, cgiState, event);
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
        catch (const std::exception &e)
        {
            std::cerr << "Connection read error: " << e.what() << std::endl;
            HttpResponse errorResp = this->createErrorResponse(400, "Bad Request", server->getConfig());
            conn.writeData(errorResp.toString());
            reactor.removeConnection(event.fd);
            return;
        }

        // Process complete requests
        if (conn.isRequestComplete())
        {
            processHttpRequest(conn, server, event);
        }
    }
    catch (const std::runtime_error &e)
    {
        std::string msg = e.what();
        if (msg.find("Connection not found") != std::string::npos)
        {
            // Connection was cleaned up - this is normal during timeout
            return;
        }
        std::cerr << "Event processing error: " << e.what() << std::endl;
    }
}

void Webserv::handleErrorEvent(const Event &event)
{
    std::cerr << "Connection error on fd " << event.fd << std::endl;
    reactor.removeConnection(event.fd);
}

void Webserv::processHttpRequest(Connection &conn, HttpServer *server, const Event &event)
{
    HttpRequest &req = conn.getCurrentRequest();
    std::string cgiEnv = ""; // Default empty CGI environment

    try
    {
        // Route matching - CREATE ROUTER PER REQUEST
        Router router;
        const RouteConfig *route = router.match(req, server->getConfig());
        HttpResponse resp;

        if (route)
        {
            // Check if request should be handled by CGI
            CgiHandler cgi(*server, req, *route, event.fd, cgiEnv);

            if (cgi.IsCgi())
            {
                handleCgiRequest(conn, cgi, server->getConfig());
                return;
            }
            else
            {
                // Handle regular HTTP request - CREATE DISPATCHER PER REQUEST
                RequestDispatcher dispatcher;
                resp = dispatcher.dispatch(req, *route, server->getConfig());
            }
        }
        else
        {
            resp = this->createErrorResponse(404, "Not Found", server->getConfig());
        }

        // Send response and handle connection persistence
        handleHttpResponse(conn, resp, req);
    }
    catch (const std::runtime_error &e)
    {
        handleHttpException(conn, server, e);
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

// ========================= EVENT HANDLERS =========================
void handleErrorEvent(const Event &event)
{
    std::string errorMsg = "Connection error on fd " + Utils::toString(event.fd) + ": ";

    if (event.errorType & POLLERR)
    {
        errorMsg += "Socket error (POLLERR)";
    }
    if (event.errorType & POLLNVAL)
    {
        errorMsg += "Invalid file descriptor (POLLNVAL)";
    }

    // if (g_reactor)
    // {
    //     g_reactor->removeConnection(event.fd);
    // }
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

void Webserv::handleCgiState(Connection &conn, CgiState *cgiState, const Event &event)
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

// ========================= HELPER FUNCTIONS =========================

void Webserv::processHttpRequest(Connection &conn, HttpServer *server,
                                 const Event &event)
{
    HttpRequest &req = conn.getCurrentRequest();
    std::string cgiEnv = ""; // Default empty CGI environment

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
                handleCgiRequest(conn, cgi, server->getConfig());
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
            resp = this->createErrorResponse(404, "Not Found", server->getConfig());
        }

        // Send response and handle connection persistence
        handleHttpResponse(conn, resp, req);
    }
    catch (const std::runtime_error &e)
    {
        handleHttpException(conn, server, e);
    }
    // catch (const std::exception &e)
    // {
    //     Error::logs("Connection error: " + std::string(e.what()));
    //     reactor.removeConnection(event.fd);
    // }
}

void Webserv::handleCgiRequest(Connection &conn, CgiHandler &cgi, const ServerConfig &config)
{
    conn.setCgiState(cgi.execCgi());

    if (conn.getCgiState())
    {
        reactor.watchCgi(&conn);
    }
    else
    {
        Error::logs("CGI execution failed");
        HttpResponse resp = this->createErrorResponse(500, "Internal Server Error", config);
        conn.writeData(resp.toString());
        reactor.removeConnection(conn.getFd());
    }
}

void Webserv::handleHttpResponse(Connection &conn, HttpResponse &resp,
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

void Webserv::handleHttpException(Connection &conn, HttpServer *server,
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

HttpResponse Webserv::createErrorResponse(int statusCode, const std::string &statusText, const ServerConfig &ServerConfig)
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
