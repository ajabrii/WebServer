/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/22 10:34:50 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/22 10:43:56 by ajabri           ###   ########.fr       */
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
#include "../includes/Webserv.hpp"
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

void Webserv::run(Reactor &reactor, const std::string &cgiEnv, char **envp, char **av)
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

                    if (event.isNewConnection)
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
                                    perror("write to CGI stdin failed");
                                close(cgiState->input_fd);
                                cgiState->input_fd = -1;
                                cgiState->bodySent = true;
                                std::cout << "\033[1;34m[CGI]\033[0m Body sent to CGI script for fd: " << event.fd << std::endl;
                                // exit(1);
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
                                    }
                                    else
                                    {
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
                                bool keepAlive = Utils::shouldKeepAlive(req);

                                if (conn.getRequestCount() >= REQUEST_LIMIT_PER_CONNECTION)
                                    keepAlive = false;
                                Utils::setConnectionHeaders(resp, keepAlive);
                                conn.writeData(resp.toString());
                                conn.reset();
                                conn.updateLastActivity();
                                if (keepAlive)
                                {
                                    conn.setKeepAlive(true);
                                    conn.incrementRequestCount();
                                    conn.resetForNextRequest();
                                    conn.updateLastActivity();
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
                                    Utils::setConnectionHeaders(errorResp, false);
                                    conn.writeData(errorResp.toString());
                                    conn.updateLastActivity();
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
                } // End of for loop
            } catch (const std::exception& e) {
                std::cerr << "Event loop error: " << e.what() << std::endl;
            }
        }
}