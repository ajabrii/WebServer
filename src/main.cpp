#include "../includes/ConfigInterpreter.hpp"
#include "../includes/HttpServer.hpp"
#include "../includes/Reactor.hpp"
#include "../includes/Router.hpp"
#include "../includes/RequestDispatcher.hpp"
#include "../includes/CgiHandler.hpp"
#include "../includes/Errors.hpp"

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

    while (std::getline(headerStream, line)) 
    {
        if (line.back() == '\r')
            line.pop_back();
        size_t colon = line.find(':');
        if (colon != std::string::npos) 
        {
            std::string key = line.substr(0, colon);
            std::string value = line.substr(colon + 1);
            while (!value.empty() && value.front() == ' ')
                value.erase(value.begin());
            response.headers[key] = value;
        }
    }

    response.statusCode = 200;
    response.statusText = "OK";
    response.body = bodyPart;
    if (response.headers.find("Content-Length") == response.headers.end())
        response.headers["Content-Length"] = std::to_string(response.body.size());

    return response;
}

void cleanupCgi(Connection &conn) 
{
    if (!conn.cgi)
        return;
    close(conn.cgi->output_fd);
    if (conn.cgi->input_fd != -1) 
        close(conn.cgi->input_fd);
    waitpid(conn.cgi->pid, NULL, 0);
    delete conn.cgi;
    conn.cgi = nullptr;
}




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

        while (true) 
        {
            reactor.poll();
            std::vector<Event> readyEvents = reactor.getReadyEvents();

            for (size_t i = 0; i < readyEvents.size(); ++i) 
            {
                Event event = readyEvents[i];

                if (event.isNewConnection) 
                {
                    HttpServer* server = reactor.getServerByListeningFd(event.fd);
                    if (server) 
                    {
                        Connection* conn = new Connection(server->acceptConnection(event.fd)); // pass listening fd
                        reactor.addConnection(conn, server);
                        std::cout << "\033[1;32m[+]\033[0m New client connected" << std::endl;
                    }
                }
                else
                {
                    Connection& conn = reactor.getConnection(event.fd);

                    if (conn.cgi && (event.fd == conn.cgi->output_fd || event.fd == conn.cgi->input_fd)) 
                    {
                        // DEBUG: Handle CGI output/input print 
                        std::cout << "\033[1;34m[CGI]\033[0m Handling CGI for fd: " << event.fd << std::endl;

                        if (event.fd == conn.cgi->input_fd && event.isWritable && !conn.cgi->pendingBody.empty()) 
                        {
                            std::cout << "\033[1;34m[CGI]\033[0m Writing pending body to CGI input" << std::endl;
                            ssize_t written = write(conn.cgi->input_fd, conn.cgi->pendingBody.c_str(), conn.cgi->pendingBody.size());
                            if (written > 0)
                            {
                                std::cout << "\033[1;34m[CGI]\033[0m Wrote " << written << " bytes to CGI input" << std::endl;
                                conn.cgi->pendingBody.erase(0, written);
                                if (conn.cgi->pendingBody.empty()) 
                                {
                                    std::cout << "\033[1;34m[CGI]\033[0m Finished writing pending body to CGI input" << std::endl;
                                    close(conn.cgi->input_fd);
                                    reactor.unregisterFd(conn.cgi->input_fd);
                                }
                            }
                        }

                        if (event.fd == conn.cgi->output_fd && event.isReadable) 
                        {
                            std::cout << "\033[1;34m[CGI]\033[0m Reading CGI output" << std::endl;
                            char buffer[4096];
                            ssize_t n = read(conn.cgi->output_fd, buffer, sizeof(buffer));
                            if (n > 0) 
                            {
                                std::cout << "\033[1;34m[CGI]\033[0m Read " << n << " bytes from CGI output" << std::endl;
                                conn.cgi->rawOutput.append(buffer, n);
                            } 
                            else if (n == 0) 
                            {
                                std::cout << "\033[1;34m[CGI]\033[0m CGI output closed" << std::endl;
                                // CGI finished
                                std::cout << "\033[1;33m[CGI OUTPUT RAW]\033[0m\n" << conn.cgi->rawOutput << std::endl;
                                HttpResponse resp = parseCgiOutput(conn.cgi->rawOutput);
                                conn.writeData(resp.toString());
                                reactor.unregisterFd(conn.cgi->output_fd);
                                cleanupCgi(conn);
                                reactor.removeConnection(conn.getFd());
                                std::cout << "\033[1;31m[-]\033[0m Connection closed (CGI done)" << std::endl;
                            } 
                            else
                            {
                                std::cerr << "\033[1;31m[CGI ERROR]\033[0m Failed to read CGI output: ";
                                perror("CGI read error");
                                reactor.unregisterFd(conn.cgi->output_fd);
                                cleanupCgi(conn);
                                reactor.removeConnection(conn.getFd());
                            }
                        }
                        continue;
                    }
                    if (event.isReadable) 
                    {
                        std::string data = conn.readData();
                        if (!data.empty()) 
                        {
                            std::cout << "\033[1;36m[>] Received:\033[0m\n" << data << std::endl;

                            HttpRequest req = HttpRequest::parse(data);
                            HttpServer* server = reactor.getServerForClient(event.fd);
                            if (!server) 
                            {
                                std::cerr << "Error: No server found for client fd: " << event.fd << std::endl;
                                continue;
                            }

                            Router router;
                            const RouteConfig* route = router.match(req, server->getConfig());
                            HttpResponse response;

                            if (route) 
                            {
                                CgiHandler cgi(*server, req, *route, event.fd, cgiEnv);
                                if (cgi.IsCgi()) 
                                {
                                    conn.cgi = cgi.execCgi();
                                    if (conn.cgi) 
                                    {
                                        conn.cgi->pendingBody = req.body;
                                        reactor.watchCgi(&conn);
                                        continue; // Wait for CGI output next poll
                                    }
                                } 
                                else
                                {
                                    RequestDispatcher dispatcher;
                                    response = dispatcher.dispatch(req, *route);
                                }
                            } 
                            else
                            {
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