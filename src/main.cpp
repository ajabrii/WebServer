/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: baouragh <baouragh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/11 13:36:53 by ajabri            #+#    #+#             */
/*   Updated: 2025/08/05 18:58:11 by baouragh         ###   ########.fr       */
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
#include "../includes/CookieParser.hpp"
#include "../includes/SessionManager.hpp"
#include "../includes/SessionID.hpp"
#include <sstream>
#include <algorithm>
#include <cctype>
#include <signal.h>
#include <cstdlib>
#define REQUEST_LIMIT_PER_CONNECTION 100
#define DEFAULT_CONFIG_PATH "./config/default.conf"


static volatile bool g_shutdown = false;
static std::vector<HttpServer *> *g_servers = NULL;
static Reactor *g_reactor = NULL;

void signalHandler(int signum)
{
    (void) signum;
    g_shutdown = true;
}

void handleErrorEvent(const Event &event)
{
    std::string errorMsg = "Connection error on fd " + Utils::toString(event.fd) + ": ";

    if (event.errorType & POLLHUP)
    {
        errorMsg += "Client disconnected (POLLHUP)";
    }
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

int main(int ac, char **av, char **envp)
{
    std::string configPath;
    if (ac == 1)
    {
        configPath = DEFAULT_CONFIG_PATH;
        std::cout << "\033[1;33m[INFO]\033[0m No config file provided. Using default path: " << configPath << std::endl;
    }
    else if (ac == 2)
    {
        configPath = av[1];
    }
    else
    {
        Error::logs("Usage: ./webserv [configuration file]");
        return 1;
    }
    
    std::vector<HttpServer *> servers;
    g_servers = &servers;
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    signal(SIGPIPE, SIG_IGN);

    try
    {
        ConfigInterpreter parser;
        parser.getConfigData(configPath);
        parser.parse();
        parser.checkValues();
        std::string cgiEnv = parser.getPathForCGI(envp);
        std::vector<ServerConfig> configs = parser.getServerConfigs();
        for (size_t i = 0; i < configs.size(); ++i)
        {
            HttpServer *server = new HttpServer(configs[i]);
            try
            {
                server->setup();
                servers.push_back(server);
            }
            catch (const std::exception &e)
            {
                delete server;
                throw ;
            }
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
                        std::cerr << "\033[1;31m[!]\033[0m Error event on fd: " << event.fd << std::endl;
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