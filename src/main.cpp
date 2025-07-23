/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/11 13:36:53 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/23 09:35:38 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/ConfigInterpreter.hpp"
#include "../includes/HttpServer.hpp"
#include "../includes/Reactor.hpp"
#include "../includes/Router.hpp"
#include "../includes/RequestDispatcher.hpp"
#include "../includes/CgiHandler.hpp"
#include "../includes/Errors.hpp"
#include "../includes/Utils.hpp"
#include <sstream>
#include <algorithm>
#include <cctype>
#include <signal.h>
#include <cstdlib>

#define REQUEST_LIMIT_PER_CONNECTION 100

// ========================= GLOBAL VARIABLES =========================
static volatile bool g_shutdown = false;
static std::vector<HttpServer *> *g_servers = NULL;
static Reactor *g_reactor = NULL;
// ========================= FUNCTION DECLARATIONS =========================
HttpResponse createNotFoundResponse(const ServerConfig &config);
HttpResponse createErrorResponse(int statusCode, const std::string &statusText, const ServerConfig &ServerConfig);
void processReadableEvent(Reactor &reactor, const Event &event, const std::string &cgiEnv);
void processHttpRequest(Reactor &reactor, Connection &conn, HttpServer *server,
                        const Event &event, const std::string &cgiEnv);
void handleCgiRequest(Reactor &reactor, Connection &conn, CgiHandler &cgi, const ServerConfig &ServerConfig);
void handleHttpResponse(Reactor &reactor, Connection &conn, HttpResponse &resp,
                        const HttpRequest &req);
void handleHttpException(Reactor &reactor, Connection &conn, HttpServer *server,
                         const std::runtime_error &e);

// ========================= SIGNAL HANDLER =========================
void signalHandler(int signum)
{
    (void)signum;
    g_shutdown = true;
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

    if (g_reactor)
    {
        g_reactor->removeConnection(event.fd);
    }
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

// ========================= MAIN FUNCTION =========================
int main(int ac, char **av, char **envp)
{
    // Validate command line arguments
    if (ac != 2)
    {
        Error::logs("Usage: " + std::string(av[0]) + " <config_file>");
        return 1;
    }

    try
    {
        // ==================== INITIALIZATION PHASE ====================

        // Parse configuration
        ConfigInterpreter parser;
        parser.getConfigData(av[1]);
        parser.parse();
        parser.checkValues();
        std::string cgiEnv = parser.getPathForCGI(envp);
        std::cout << "[✔] Config loaded" << std::endl;

        // Initialize servers
        std::vector<ServerConfig> configs = parser.getServerConfigs();
        std::vector<HttpServer *> servers;

        // Setup signal handlers
        signal(SIGINT, signalHandler);
        signal(SIGTERM, signalHandler);
        g_servers = &servers;

        // Create and setup HTTP servers
        for (size_t i = 0; i < configs.size(); ++i)
        {
            HttpServer *server = new HttpServer(configs[i]);
            server->setup();
            servers.push_back(server);
        }

        // Initialize reactor and register servers
        Reactor reactor;
        g_reactor = &reactor;
        for (size_t i = 0; i < servers.size(); ++i)
        {
            reactor.registerServer(*servers[i]);
        }

        // ==================== MAIN EVENT LOOP ====================

        while (!g_shutdown)
        {
            try
            {
                // Poll for events and cleanup expired connections
                reactor.poll();
                reactor.cleanupTimedOutConnections();

                // Process all ready events
                std::vector<Event> events = reactor.getReadyEvents();

                for (size_t i = 0; i < events.size(); ++i)
                {
                    Event event = events[i];

                    // ============== EVENT PROCESSING ==============

                    if (event.isNewConnection)
                    {
                        handleNewConnection(reactor, event);
                    }
                    else if (event.isReadable || event.isPullHUP)
                    {
                        processReadableEvent(reactor, event, cgiEnv);
                    }
                    else if (event.isError)
                    {
                        handleErrorEvent(event);
                    }
                }
            }
            catch (const std::exception &e)
            {
                std::cerr << "Event loop error: " << e.what() << std::endl;
            }
        }

        // ==================== CLEANUP PHASE ====================

        std::cout << "\n[INFO] Shutting down gracefully..." << std::endl;
        reactor.cleanup();

        for (size_t i = 0; i < servers.size(); ++i)
        {
            delete servers[i];
        }
        servers.clear();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fatal: " << e.what() << std::endl;
        if (g_servers)
        {
            for (size_t i = 0; i < g_servers->size(); ++i)
            {
                delete (*g_servers)[i];
            }
        }
        return 1;
    }

    return 0;
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
            // No route found - return 404
            resp = createNotFoundResponse(server->getConfig());
        }

        // Send response and handle connection persistence
        handleHttpResponse(reactor, conn, resp, req);
    }
    catch (const std::runtime_error &e)
    {
        handleHttpException(reactor, conn, server, e);
    }
    catch (const std::exception &e)
    {
        Error::logs("Connection error: " + std::string(e.what()));
        reactor.removeConnection(event.fd);
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

    // Handle incomplete body (continue reading)
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
HttpResponse createNotFoundResponse(const ServerConfig &config)
{
    HttpResponse resp;
    resp.version = "HTTP/1.1";
    resp.statusCode = 404;
    resp.statusText = "Not Found";
    resp.headers["content-type"] = "text/html";
    resp.body = Error::loadErrorPage(404, config);
    resp.headers["content-length"] = Utils::toString(resp.body.size());
    return resp;
}

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
