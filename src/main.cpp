/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/11 13:36:53 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/14 14:56:05 by ajabri           ###   ########.fr       */
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
    
    Error::logs(errorMsg);
    
    // Remove the connection immediately - this will clean up resources
    if (g_reactor) {
        g_reactor->removeConnection(event.fd);
    }
}

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

        // Setup signal handlers for graceful shutdown
        signal(SIGINT, signalHandler);
        signal(SIGTERM, signalHandler);
        g_servers = &servers;

        //* === Setup servers === (done 100%)
        for (size_t i = 0; i < configs.size(); ++i) {
            HttpServer* server = new HttpServer(configs[i]);
            server->setup();
            servers.push_back(server);
        }

        //* === Setup the multiplixing monitor aka reactor === (done 99.99%)
        Reactor reactor;
        g_reactor = &reactor;
        for (size_t i = 0; i < servers.size(); ++i)
            reactor.registerServer(*servers[i]);

        //* === Event loop ===
        while (!g_shutdown)
        {
            try {
                reactor.poll(); //? hna kan3mer wa7d struct smitha Event ghatl9awha f reactor class
                
                // Cleanup timed-out connections periodically
                reactor.cleanupTimedOutConnections();
                
                std::vector<Event> events = reactor.getReadyEvents(); //? Hna kangeti dik struct li fiha evensts li 3mrathom poll (kernel li 3merhom poll it's system call you for more infos go to reactor.cpp > void Reactor::poll())

                //? hna kanlopi ela ga3 events struct kola wahd o chno khasni ndir lih/bih isnewconnection isReadble (POLLIN) isWritble
                for (size_t i = 0; i < events.size(); ++i)
            {
                Event event = events[i];

                // Handle error events FIRST (highest priority)
                if (event.isError)
                {
                    handleErrorEvent(event);
                }
                else if (event.isNewConnection)
                {
                    HttpServer* server = reactor.getServerByListeningFd(event.fd); //? hna kanchof ina server t connecta m3ah l client bach nchof ina route khsni nmchi lih mn ber3d
                    if (server)
                    {
                        Connection* conn = new Connection(server->acceptConnection(event.fd)); //? hna kan 9ad wahd object dial connection kan constructih b client object li kay creah (acceptih) server
                        conn->updateLastActivity(); // Initialize activity timestamp for new connection
                        reactor.addConnection(conn, server);
                        std::cout << NEW_CLIENT_CON << std::endl;
                    }
                }
                else if (event.isReadable)
                {
                    try {
                        Connection& conn = reactor.getConnection(event.fd); //@ kangeti dak l connection li tcreat (katkon fmap)
                        
                        // Get the server for this specific client connection
                        HttpServer* server = reactor.getServerForClient(event.fd);
                        if (!server) {
                            Error::logs("Error: No server found for client connection");
                            reactor.removeConnection(event.fd);
                            continue;
                        }
                        
                        size_t maxBodySize = server->getConfig().clientMaxBodySize;
                        
                        // Check if buffer size exceeds the limit (protection against buffer overflow)
                        if (conn.getBuffer().size() > maxBodySize) {
                            Error::logs("Request buffer exceeds maximum size limit");
                            HttpResponse resp;
                            resp.version = "HTTP/1.1";
                            resp.statusCode = 413;
                            resp.statusText = "Payload Too Large";
                            resp.body = Error::loadErrorPage(413, server->getConfig());
                            std::stringstream ss;
                            ss << resp.body.size();
                            resp.headers["content-length"] = ss.str();
                            resp.headers["content-type"] = "text/html";
                            
                            conn.writeData(resp.toString());
                            reactor.removeConnection(event.fd);
                            continue;
                        }
                        
                        std::string data = conn.readData(); //@ (request li kaysifto l kliyan)This reads new data and accumulates in buffer
                        conn.updateLastActivity(); // Update activity timestamp when receiving data from client
                        
                        //@ Check if we have complete headers hna fin kantsna ywselni request camel
                        size_t headerEnd = conn.getBuffer().find("\r\n\r\n");
                        if (headerEnd == std::string::npos) {
                            // Check for oversized headers (security protection)
                            if (conn.getBuffer().size() > 8192) { // 8KB header limit
                                Error::logs("Header too large - potential attack");
                                HttpResponse resp;
                                resp.statusCode = 431;
                                resp.statusText = "Request Header Fields Too Large";
                                resp.headers["content-type"] = "text/html";
                                resp.body = Error::loadErrorPage(431, server->getConfig());
                                std::stringstream ss;
                                ss << resp.body.size();
                                resp.headers["content-length"] = ss.str();
                                
                                conn.writeData(resp.toString());
                                reactor.removeConnection(event.fd);
                                continue;
                            }
                            continue;
                        }
                    //@ Headers are complete, now check if body is complete (for POST requests)
                    std::string headerPart = conn.getBuffer().substr(0, headerEnd + 4);
                    std::string remainingData = conn.getBuffer().substr(headerEnd + 4);

                    size_t contentLength = 0;
                    // Make header search case-insensitive
                    std::string headerLower = headerPart;
                    std::transform(headerLower.begin(), headerLower.end(), headerLower.begin(), ::tolower);
                    
                    size_t clPos = headerLower.find("content-length:");
                    if (clPos != std::string::npos)
                    {
                        size_t clStart = headerPart.find(":", clPos) + 1;
                        size_t clEnd = headerPart.find("\r\n", clStart);
                        if (clEnd != std::string::npos)
                        {
                            std::string clStr = headerPart.substr(clStart, clEnd - clStart);
                            clStr.erase(0, clStr.find_first_not_of(" \t"));
                            clStr.erase(clStr.find_last_not_of(" \t") + 1);
                            
                            // C++98 compatible conversion instead of std::stoul
                            std::stringstream ss(clStr);
                            ss >> contentLength;
                            if (ss.fail()) {
                                Error::logs("Invalid Content-Length header: " + clStr);
                                contentLength = 0;
                            }
                        }
                    }
                    
                    // Check if the declared content length exceeds the maximum allowed body size
                    if (contentLength > maxBodySize)
                    {
                        Error::logs("Request body exceeds maximum size limit");
                        HttpResponse resp;
                        resp.version = "HTTP/1.1";
                        resp.statusCode = 413;
                        resp.statusText = "Payload Too Large";
                        resp.body = Error::loadErrorPage(413, server->getConfig());
                        
                        // C++98 compatible string conversion
                        std::stringstream ss;
                        ss << resp.body.size();
                        resp.headers["content-length"] = ss.str();
                        resp.headers["content-type"] = "text/html";
                        
                        conn.writeData(resp.toString());
                        reactor.removeConnection(event.fd);
                        continue; // Important: continue to next event
                    }
                        
                    if (contentLength > 0 && remainingData.size() < contentLength)
                        continue;
                    std::cout << RECEV_COMPLETE << std::endl;

                    try
                    {
                        HttpRequest req = HttpRequest::parse(conn.getBuffer());
                        conn.clearBuffer();
                        HttpServer* server = reactor.getServerForClient(event.fd);
                        if (!server) {
                            Error::logs("Error: No server found");
                            reactor.removeConnection(event.fd);
                            continue;
                        }

                        Router router;
                        const RouteConfig* route = router.match(req, server->getConfig());
                        HttpResponse resp;

                        if (route) {
                            CgiHandler cgi(*server, req, *route, event.fd, cgiEnv);
                            if (cgi.IsCgi()) {
                                resp = cgi.execCgi();
                            } else {
                                RequestDispatcher dispatcher;
                                resp = dispatcher.dispatch(req, *route, server->getConfig());
                            }
                        } else {
                            // No route found - return 404
                            resp.version = "HTTP/1.1";  // Fix: Set HTTP version
                            resp.statusCode = 404;
                            resp.statusText = "Not Found";
                            resp.headers["content-type"] = "text/html";
                            resp.body = Error::loadErrorPage(404, server->getConfig());
                            
                            // C++98 compatible string conversion
                            std::stringstream ss;
                            ss << resp.body.size();
                            resp.headers["content-length"] = ss.str();
                            
                            std::cout << "\033[1;33m[Main]\033[0m No route found for URI: " << req.uri << " - returning 404" << std::endl;
                        }

                        // Determine if we should keep the connection alive
                        bool keepAlive = shouldKeepAlive(req);
                        
                        // Limit number of requests per connection to prevent resource exhaustion
                        if (conn.getRequestCount() >= REQUEST_LIMIT_PER_CONNECTION)
                            keepAlive = false;
                        // Set appropriate connection headers
                        setConnectionHeaders(resp, keepAlive);
                        
                        // Send the response
                        conn.writeData(resp.toString());
                        conn.updateLastActivity(); // Update activity timestamp after sending response
                        
                        // Handle connection based on keep-alive decision
                        if (keepAlive) {
                            conn.setKeepAlive(true);
                            conn.incrementRequestCount();
                            conn.resetForNextRequest();
                            conn.updateLastActivity(); // Reset timeout for keep-alive connection
                            std::cout << "\033[1;32m[+]\033[0m Connection kept alive (request #" << conn.getRequestCount() << ")" << std::endl;
                        } else {
                            reactor.removeConnection(event.fd);
                            std::cout << "\033[1;31m[-]\033[0m Connection closed" << std::endl;
                        }
                    }
                    catch (const std::runtime_error& e) {
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
                            
                            // C++98 compatible string conversion
                            std::stringstream ss;
                            ss << errorResp.body.size();
                            errorResp.headers["content-length"] = ss.str();
                            
                            // Always close connection on parse errors
                            setConnectionHeaders(errorResp, false);
                            conn.writeData(errorResp.toString());
                            conn.updateLastActivity(); // Update activity timestamp after error response
                            reactor.removeConnection(event.fd);
                        }
                    } catch (const std::exception& e) {
                        Error::logs("Connection error: " + std::string(e.what()));
                        reactor.removeConnection(event.fd);
                    }
                    } catch (const std::exception& e) {
                        Error::logs("Connection read error: " + std::string(e.what()));
                        reactor.removeConnection(event.fd);
                    }
                }
            } // End of for loop
            } catch (const std::exception& e) {
                std::cerr << "Event loop error: " << e.what() << std::endl;
                // Continue with next iteration
            }
        } // End of while loop

        // === Graceful cleanup ===
        std::cout << "\n[INFO] Shutting down gracefully..." << std::endl;
        
        // Clean up all connections in reactor
        reactor.cleanup();
        
        // Clean up servers
        for (size_t i = 0; i < servers.size(); ++i) {
            delete servers[i];
        }
        servers.clear();

    } catch (const std::exception& e) {
        std::cerr << "Fatal: " << e.what() << std::endl;
        
        // Emergency cleanup
        if (g_servers) {
            for (size_t i = 0; i < g_servers->size(); ++i) {
                delete (*g_servers)[i];
            }
        }
        return 1;
    }

    return 0;
}
