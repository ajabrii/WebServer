#include "../includes/ConfigInterpreter.hpp"
#include "../includes/HttpServer.hpp"
#include "../includes/Reactor.hpp"
#include "../includes/Router.hpp"
#include "../includes/RequestDispatcher.hpp"
#include "../includes/CgiHandler.hpp"
#include "../includes/Errors.hpp"

int main(int ac, char **av, char **envp) {
    if (ac != 2) {
        // std::cerr <<  << std::endl;
        Error::logs("Usage: " + std::string(av[0]) + " <config_file>");
        return 1;
    }

    try {
        ConfigInterpreter parser;
        parser.getConfigData(av[1]);
        parser.parse();
        parser.checkValues();
        std::string cgiEnv = parser.getPathForCGI(envp);
        std::cout << "[✔] Config loaded successfully. CGI path: " << cgiEnv << std::endl;

        std::vector<ServerConfig> configs = parser.getServerConfigs();
        std::vector<HttpServer*> servers;

        // Create & setup servers
        for (size_t i = 0; i < configs.size(); ++i) {
            HttpServer* server = new HttpServer(configs[i]);
            server->setup();
            servers.push_back(server);
        }

        Reactor reactor;
        for (size_t i = 0; i < servers.size(); ++i) {
            reactor.registerServer(*servers[i]);
        }

        while (true) {
            reactor.poll();
            std::vector<Event> readyEvents = reactor.getReadyEvents();

            for (size_t i = 0; i < readyEvents.size(); ++i) {
                Event event = readyEvents[i];

                if (event.isNewConnection) {
                    HttpServer* server = reactor.getServerByListeningFd(event.fd);
                    if (server) {
                        Connection* conn = new Connection(server->acceptConnection(event.fd)); // pass listening fd
                        reactor.addConnection(conn, server);
                        std::cout << "\033[1;32m[+]\033[0m New client connected" << std::endl;
                    }
                }
                else if (event.isReadable) {
                    Connection& conn = reactor.getConnection(event.fd);
                    std::string data = conn.readData();

                    if (!data.empty()) {
                        std::cout << "\033[1;36m[>] Received:\033[0m\n" << data << std::endl;

                        HttpRequest req = HttpRequest::parse(data);
                        HttpServer* server = reactor.getServerForClient(event.fd);
                        if (!server) {
                            std::cerr << "Error: No server found for client fd: " << event.fd << std::endl;
                            continue;
                        }

                        Router router;
                        const RouteConfig* route = router.match(req, server->getConfig());
                        HttpResponse response;
                        if (route) {
                            std::cout << "event fd_----------------> " << event.fd << "\n";
                            CgiHandler cgi(*server, req, *route, event.fd, cgiEnv);
                            if (cgi.IsCgi()) {
                                response = cgi.execCgi();
                            } else {
                                RequestDispatcher dispatcher;
                                response = dispatcher.dispatch(req, *route);
                            }
                            std::cout << "[<] Response: " << response.statusCode << " " << response.statusText << std::endl;
                        } else {
                            response.statusCode = 404;
                            response.statusText = "Not Found";
                            response.body = Error::loadErrorPage(404, server->getConfig());
                            response.headers["Content-Length"] = std::to_string(response.body.size());
                        }

                        conn.writeData(response.toString());
                        reactor.removeConnection(event.fd);
                        std::cout << "\033[1;31m[-]\033[0m Connection closed" << std::endl;
                    }
                }
            }
        }

        // Cleanup (never reached)
        for (size_t i = 0; i < servers.size(); ++i)
            delete servers[i];

    } catch (const std::exception& e) {
        std::cerr << "Fatal: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}