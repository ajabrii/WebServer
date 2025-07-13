/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/11 13:36:53 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/12 18:23:26 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/ConfigInterpreter.hpp"
#include "../includes/HttpServer.hpp"
#include "../includes/Reactor.hpp"
#include "../includes/Router.hpp"
#include "../includes/RequestDispatcher.hpp"
#include "../includes/CgiHandler.hpp"
#include "../includes/Errors.hpp"

int main(int ac, char **av, char **envp) {
    if (ac != 2) {
        Error::logs("Usage: " + std::string(av[0]) + " <config_file>");
        return 1;
    }

    try {
        // === Load & parse config ===
        ConfigInterpreter parser;
        parser.getConfigData(av[1]);
        parser.parse();
        parser.checkValues();
        std::string cgiEnv = parser.getPathForCGI(envp);
        std::cout << "[✔] Config loaded. CGI path: " << cgiEnv << std::endl;

        std::vector<ServerConfig> configs = parser.getServerConfigs();
        std::vector<HttpServer*> servers;

        // === Setup servers ===
        for (size_t i = 0; i < configs.size(); ++i) {
            HttpServer* server = new HttpServer(configs[i]);
            server->setup();
            servers.push_back(server);
        }

        Reactor reactor;
        for (size_t i = 0; i < servers.size(); ++i)
            reactor.registerServer(*servers[i]);

        // === Event loop ===
        while (true)
        {
            reactor.poll();
            std::vector<Event> events = reactor.getReadyEvents();

            for (size_t i = 0; i < events.size(); ++i) {
                Event event = events[i];

                if (event.isNewConnection) {
                    HttpServer* server = reactor.getServerByListeningFd(event.fd);
                    if (server) {
                        Connection* conn = new Connection(server->acceptConnection(event.fd));
                        reactor.addConnection(conn, server);
                        std::cout << "\033[1;32m[+]\033[0m New client connected" << std::endl;
                    }
                }
                else if (event.isReadable) {
                    Connection& conn = reactor.getConnection(event.fd);
                    std::string data = conn.readData(); // This reads new data and accumulates in buffer
                    std::cout << data << std::endl;
                    // Debug: Show current buffer state
                    // std::cout << "\033[1;35m[DEBUG] Current buffer size: " << conn.getBuffer().size() << " bytes\033[0m" << std::endl;
                    
                    // Check if we have complete headers
                    size_t headerEnd = conn.getBuffer().find("\r\n\r\n");
                    if (headerEnd == std::string::npos) {
                        // std::cout << "\033[1;33m[!] Incomplete headers, waiting for more data...\033[0m" << std::endl;
                        continue;
                    }
                    
                    // Headers are complete, now check if body is complete (for POST requests)
                    std::string headerPart = conn.getBuffer().substr(0, headerEnd + 4);
                    std::string remainingData = conn.getBuffer().substr(headerEnd + 4);
                    
                    size_t contentLength = 0;
                    size_t clPos = headerPart.find("Content-Length:");
                    if (clPos != std::string::npos) {
                        size_t clStart = headerPart.find(":", clPos) + 1;
                        size_t clEnd = headerPart.find("\r\n", clStart);
                        if (clEnd != std::string::npos) {
                            std::string clStr = headerPart.substr(clStart, clEnd - clStart);
                            // Trim whitespace
                            clStr.erase(0, clStr.find_first_not_of(" \t"));
                            clStr.erase(clStr.find_last_not_of(" \t") + 1);
                            contentLength = std::stoul(clStr);
                        }
                    }
                    // Check if we have the complete body
                    if (contentLength > 0 && remainingData.size() < contentLength) {
                        // std::cout << "\033[1;33m[!] Incomplete body (" << remainingData.size() 
                        //         //   << "/" << contentLength << " bytes), waiting for more data...\033[0m" << std::endl;
                        continue;
                    }
                    
                    std::cout << "\033[1;36m[>] Received complete request:\033[0m\n" << std::endl;

                    try {
                        HttpRequest req = HttpRequest::parse(conn.getBuffer());
                        conn.clearBuffer(); // parsed successfully

                        HttpServer* server = reactor.getServerForClient(event.fd);
                        if (!server) {
                            std::cerr << "Error: No server found for fd: " << event.fd << std::endl;
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
                            resp.statusCode = 404;
                            resp.statusText = "Not Found";
                            resp.body = Error::loadErrorPage(404, server->getConfig());
                            resp.headers["Content-Length"] = std::to_string(resp.body.size());
                        }

                        conn.writeData(resp.toString());
                        reactor.removeConnection(event.fd);
                        std::cout << "\033[1;31m[-]\033[0m Connection closed" << std::endl;
                    }
                    catch (const std::runtime_error& e) {
                        std::string msg = e.what();
                        if (msg.find("incomplete body") != std::string::npos) {
                            // This should not happen now due to our pre-check above
                            // std::cout << "\033[1;33m[!] Unexpected incomplete body error, continuing...\033[0m" << std::endl;
                            continue;
                        } else {
                            std::cerr << "Parse error: " << msg << std::endl;
                            HttpResponse errorResp;
                            errorResp.statusCode = 400;
                            errorResp.statusText = "Bad Request";
                            errorResp.body = "HTTP/1.1 400 Bad Request\r\n\r\nBad Request";
                            errorResp.headers["Content-Length"] = std::to_string(errorResp.body.size());
                            conn.writeData(errorResp.toString());
                            reactor.removeConnection(event.fd);
                        }
                    }
                }
            }
        }

        // === Cleanup (never reached) ===
        for (size_t i = 0; i < servers.size(); ++i)
            delete servers[i];

    } catch (const std::exception& e) {
        std::cerr << "Fatal: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
