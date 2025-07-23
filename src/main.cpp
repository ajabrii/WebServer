/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/11 13:36:53 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/23 11:36:50 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/ConfigInterpreter.hpp"
#include "../includes/HttpServer.hpp"
#include "../includes/Utils.hpp"
#include <sstream>
#include <algorithm>
#include <cctype>
#include <signal.h>
#include <cstdlib>

static volatile bool g_shutdown = false;
static std::vector<HttpServer *> *g_servers = NULL;
static Reactor *g_reactor = NULL;

void signalHandler(int signum)
{
    (void)signum;
    g_shutdown = true;
}

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

int main(int ac, char **av, char **envp)
{
    if (ac != 2)
    {
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
        std::vector<HttpServer *> servers;

        signal(SIGINT, signalHandler);
        signal(SIGTERM, signalHandler);
        g_servers = &servers;

        for (size_t i = 0; i < configs.size(); ++i)
        {
            HttpServer *server = new HttpServer(configs[i]);
            server->setup();
            servers.push_back(server);
        }

        Reactor reactor;
        g_reactor = &reactor;
        for (size_t i = 0; i < servers.size(); ++i)
        {
            reactor.registerServer(*servers[i]);
        }
        while (!g_shutdown)
        {
            try
            {
                reactor.poll();
                std::vector<Event> events = reactor.getReadyEvents();
                for (size_t i = 0; i < events.size(); ++i)
                {
                    Event event = events[i];
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
                reactor.cleanupTimedOutConnections();
            }
            catch (const std::exception &e)
            {
                std::cerr << "Event loop error: " << e.what() << std::endl;
            }
        }
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

