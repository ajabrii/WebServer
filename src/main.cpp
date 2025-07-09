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
        while (true) {
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
                    std::string data = conn.readData();
                    conn.getBuffer() += data; // accumulate

                    try {
                        std::cout << "\033[1;31m"<< conn.getBuffer() << event.fd << std::endl;
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
                                resp = dispatcher.dispatch(req, *route);
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
                            // keep accumulating next poll
                            continue;
                        } else {
                            std::cerr << "Parse error: " << msg << std::endl;
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












// #include "../includes/ConfigInterpreter.hpp"
// #include "../includes/HttpServer.hpp"
// #include "../includes/Reactor.hpp"
// #include "../includes/Router.hpp"
// #include "../includes/RequestDispatcher.hpp"
// #include "../includes/CgiHandler.hpp"
// #include "../includes/Errors.hpp"

// int main(int ac, char **av, char **envp) {
//     if (ac != 2) {
//         // std::cerr <<  << std::endl;
//         Error::logs("Usage: " + std::string(av[0]) + " <config_file>");
//         return 1;
//     }

//     try {
//         ConfigInterpreter parser;
//         parser.getConfigData(av[1]);
//         parser.parse();
//         parser.checkValues();
//         std::string cgiEnv = parser.getPathForCGI(envp);
//         std::cout << "[✔] Config loaded successfully. CGI path: " << cgiEnv << std::endl;

//         std::vector<ServerConfig> configs = parser.getServerConfigs();
//         std::vector<HttpServer*> servers;

//         // Create & setup servers
//         for (size_t i = 0; i < configs.size(); ++i) {
//             HttpServer* server = new HttpServer(configs[i]);
//             server->setup();
//             servers.push_back(server);
//         }

//         Reactor reactor;
//         for (size_t i = 0; i < servers.size(); ++i) {
//             reactor.registerServer(*servers[i]);
//         }

//         while (true) {
//             reactor.poll();
//             std::vector<Event> readyEvents = reactor.getReadyEvents();

//             for (size_t i = 0; i < readyEvents.size(); ++i) {
//                 Event event = readyEvents[i];

//                 if (event.isNewConnection) {
//                     HttpServer* server = reactor.getServerByListeningFd(event.fd);
//                     if (server) {
//                         Connection* conn = new Connection(server->acceptConnection(event.fd)); // pass listening fd
//                         reactor.addConnection(conn, server);
//                         std::cout << "\033[1;32m[+]\033[0m New client connected" << std::endl;
//                     }
//                 }
//                 else if (event.isReadable) {
//                     Connection& conn = reactor.getConnection(event.fd);
//                     std::string data = conn.readData();

//                     if (!data.empty()) {
//                         std::cout << "\033[1;36m[>] Received:\033[0m\n" << data << std::endl;

//                         HttpRequest req = HttpRequest::parse(data);
//                         HttpServer* server = reactor.getServerForClient(event.fd);
//                         if (!server) {
//                             std::cerr << "Error: No server found for client fd: " << event.fd << std::endl;
//                             continue;
//                         }

//                         Router router;
//                         const RouteConfig* route = router.match(req, server->getConfig());
//                         HttpResponse response;
//                         if (route) {
//                             CgiHandler cgi(*server, req, *route, event.fd, cgiEnv);
//                             if (cgi.IsCgi()) {
//                                 response = cgi.execCgi();
//                             } else {
//                                 RequestDispatcher dispatcher;
//                                 response = dispatcher.dispatch(req, *route);
//                             }
//                             std::cout << "[<] Response: " << response.statusCode << " " << response.statusText << std::endl;
//                         } else {
//                             response.statusCode = 404;
//                             response.statusText = "Not Found";
//                             response.body = Error::loadErrorPage(404, server->getConfig());
//                             response.headers["Content-Length"] = std::to_string(response.body.size());
//                         }

//                         conn.writeData(response.toString());
//                         reactor.removeConnection(event.fd);
//                         std::cout << "\033[1;31m[-]\033[0m Connection closed" << std::endl;
//                     }
//                 }
//             }
//         }

//         // Cleanup (never reached)
//         for (size_t i = 0; i < servers.size(); ++i)
//             delete servers[i];

//     } catch (const std::exception& e) {
//         std::cerr << "Fatal: " << e.what() << std::endl;
//         return 1;
//     }

//     return 0;
// }