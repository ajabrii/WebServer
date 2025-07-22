/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/11 13:36:53 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/22 11:11:43 by ajabri           ###   ########.fr       */
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
# define REQUEST_LIMIT_PER_CONNECTION 100

static volatile bool g_shutdown = false;
static std::vector<HttpServer*>* g_servers = NULL;
static Reactor* g_reactor = NULL;

void signalHandler(int signum) {
    (void)signum;
    g_shutdown = true;
}

std::string toLower(const std::string& s) {
    std::string result = s;
    for (size_t i = 0; i < result.size(); ++i)
        result[i] = std::tolower(result[i]);
    return result;
}

void ltrim(std::string& s) {
    size_t i = 0;
    while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) ++i;
    s.erase(0, i);
}

HttpResponse parseCgiOutput(const std::string& raw)
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
            line.erase(line.size() - 1);

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

void handleErrorEvent(const Event& event)
{
    std::string errorMsg = "Connection error on fd " + Utils::toString(event.fd) + ": ";

    if (event.errorType & POLLHUP) {
        errorMsg += "Client disconnected (POLLHUP)";
    }
    if (event.errorType & POLLERR) {
        errorMsg += "Socket error (POLLERR)";
    }
    if (event.errorType & POLLNVAL) {
        errorMsg += "Invalid file descriptor (POLLNVAL)";
    }
    if (g_reactor) {
        g_reactor->removeConnection(event.fd);
    }
}

void handleNewConnection(Reactor &reactor, const Event &event)
{
    HttpServer *server = reactor.getServerByListeningFd(event.fd);
    if (server)
    {
        Connection* conn = new Connection(server->acceptConnection(event.fd));
        conn->updateLastActivity();
        reactor.addConnection(conn, server);
        std::cout << NEW_CLIENT_CON << std::endl;
    }
}

// void handleCgiState(Reactor& reactor, Connection& conn, CgiState* cgiState, const Event& event)
// {
//     if (!cgiState->bodySent && conn.getCurrentRequest().method == POST && cgiState->input_fd != -1)
//     {
//         ssize_t written = write(cgiState->input_fd, conn.getCurrentRequest().body.c_str(), conn.getCurrentRequest().body.size());
//         if (written == -1)
//             perror("write to CGI stdin failed");
//         close(cgiState->input_fd);
//         cgiState->input_fd = -1;
//         cgiState->bodySent = true;
//         std::cout << "\033[1;34m[CGI]\033[0m Body sent to CGI script for fd: " << event.fd << std::endl;
//     }
//     char buffer[4096];
//     ssize_t n = read(conn.getCgiState()->output_fd, buffer, sizeof(buffer));
//     if (n > 0)
//     {
//         conn.getCgiState()->rawOutput.append(buffer, n);
//     }
//     else if (n == 0)
//     {
//         HttpResponse resp = parseCgiOutput(conn.getCgiState()->rawOutput);
//         conn.writeData(resp.toString());
//         reactor.removeConnection(event.fd);
//         std::cout << "\033[1;31m[-]\033[0m Connection closed (CGI done)" << std::endl;
//     }
//     else
//     {
//         Error::logs("CGI read error on fd " + Utils::toString(event.fd));
//         reactor.removeConnection(event.fd);
//     }
// }

int main(int ac, char **av, char **envp)
{
    if (ac != 2) {
        Error::logs("Usage: " + std::string(av[0]) + " <config_file>");
        return 1;
    }

    try
    {
        ConfigInterpreter parser;
        parser.getConfigData(av[1]);
        parser.parse();
        parser.checkValues();
        std::string cgiEnv = parser.getPathForCGI(envp);

        std::cout << "[✔] Config loaded" << std::endl;

        std::vector<ServerConfig> configs = parser.getServerConfigs();
        std::vector<HttpServer*> servers;

        signal(SIGINT, signalHandler);
        signal(SIGTERM, signalHandler);
        g_servers = &servers;
        for (size_t i = 0; i < configs.size(); ++i) {
            HttpServer* server = new HttpServer(configs[i]);
            server->setup();
            servers.push_back(server);
        }
        Reactor reactor;
        g_reactor = &reactor;
        for (size_t i = 0; i < servers.size(); ++i)
            reactor.registerServer(*servers[i]);
        while (!g_shutdown)
        {
            try
            {
                reactor.poll();

                reactor.cleanupTimedOutConnections();

                std::vector<Event> events = reactor.getReadyEvents();

                for (size_t i = 0; i < events.size(); ++i)
                {
                    Event event = events[i];

                    if (event.isNewConnection){

                        handleNewConnection(reactor, event);
                    }
                    else if (event.isReadable || event.isPullHUP)
                    {
                        Connection &conn = reactor.getConnection(event.fd);
                        CgiState *cgiState = conn.getCgiState();
                        if (cgiState)
                        {
                            if (!cgiState->bodySent && conn.getCurrentRequest().method == POST && cgiState->input_fd != -1)
                            {
                                ssize_t written = write(cgiState->input_fd, conn.getCurrentRequest().body.c_str(), conn.getCurrentRequest().body.size());
                                if (written == -1)
                                    Error::logs("write to CGI stdin failed for fd " + Utils::toString(event.fd));
                                close(cgiState->input_fd);
                                cgiState->input_fd = -1;
                                cgiState->bodySent = true;
                                std::cout << "\033[1;34m[CGI]\033[0m Body sent to CGI script for fd: " << event.fd << std::endl;
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
                        else
                        {

                            HttpServer* server = reactor.getServerForClient(event.fd);
                            if (!server) {
                                reactor.removeConnection(event.fd);
                                std::cerr << "Error: No server found for client fd: " << event.fd << std::endl;
                                continue;
                            }

                            try {
                                conn.readData(server);
                            } catch (const std::exception& e) {
                                std::cerr << "Connection read error: " << e.what() << std::endl;
                                reactor.removeConnection(event.fd);
                                continue;
                            }
                           if (conn.isRequestComplete())
                           {
                                HttpRequest& req = conn.getCurrentRequest();
                                try
                                {
                                    Router router;
                                    const RouteConfig* route = router.match(req, server->getConfig());
                                    HttpResponse resp;
                                    if (route)
                                    {
                                        CgiHandler cgi(*server, req, *route, event.fd, cgiEnv);
                                        if (cgi.IsCgi())
                                        {
                                            conn.setCgiState(cgi.execCgi());
                                            if (conn.getCgiState())
                                            {
                                                reactor.watchCgi(&conn);
                                                continue;
                                            }
                                            else
                                            {
                                                Error::logs("CGI execution failed");
                                                resp.version = "HTTP/1.1";
                                                resp.statusCode = 500;
                                                resp.statusText = "Internal Server Error";
                                                resp.headers["content-type"] = "text/html";
                                                resp.body = Error::loadErrorPage(500, server->getConfig());
                                                resp.headers["content-length"] = Utils::toString(resp.body.size());
                                            }
                                        }else    {
                                            RequestDispatcher dispatcher;
                                            resp = dispatcher.dispatch(req, *route, server->getConfig());
                                        }
                                    } else {
                                        resp.version = "HTTP/1.1";
                                        resp.statusCode = 404;
                                        resp.statusText = "Not Found";
                                        resp.headers["content-type"] = "text/html";
                                        resp.body = Error::loadErrorPage(404, server->getConfig());
                                        resp.headers["content-length"] = Utils::toString(resp.body.size());

                                    }
                                    bool keepAlive = shouldKeepAlive(req);

                                    if (conn.getRequestCount() >= REQUEST_LIMIT_PER_CONNECTION)
                                        keepAlive = false;
                                    setConnectionHeaders(resp, keepAlive);
                                    conn.writeData(resp.toString());
                                    conn.reset();
                                    conn.updateLastActivity();
                                    if (keepAlive)
                                    {
                                        conn.setKeepAlive(true);
                                        conn.incrementRequestCount();
                                        conn.resetForNextRequest();
                                        std::cout << "\033[1;32m[+]\033[0m Connection kept alive (request #" << conn.getRequestCount() << ")" << std::endl;
                                    } else {
                                        reactor.removeConnection(event.fd);
                                        std::cout << "\033[1;31m[-]\033[0m Connection closed" << std::endl;
                                    }
                                }   catch (const std::runtime_error& e) {
                                    std::string msg = e.what();
                                    if (msg.find("incomplete body") != std::string::npos) {
                                        continue;
                                    } else {
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
                                        reactor.removeConnection(event.fd);
                                    }
                                    } catch (const std::exception& e) {
                                    Error::logs("Connection error: " + std::string(e.what()));
                                    reactor.removeConnection(event.fd);
                                    }
                            }
                        }
                    }
                    else if (event.isError)
                    {
                        handleErrorEvent(event);
                    }
                }
            } catch (const std::exception& e) {
                std::cerr << "Event loop error: " << e.what() << std::endl;
            }
        }

        std::cout << "\n[INFO] Shutting down gracefully..." << std::endl;
        reactor.cleanup();
        for (size_t i = 0; i < servers.size(); ++i) {
            delete servers[i];
        }
        servers.clear();
    } catch (const std::exception& e){
        std::cerr << "Fatal: " << e.what() << std::endl;
        if (g_servers) {
            for (size_t i = 0; i < g_servers->size(); ++i) {
                delete (*g_servers)[i];
            }
        }
        return 1;
    }

    return 0;
}
