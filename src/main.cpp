/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/11 13:36:53 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/14 09:40:48 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/ConfigInterpreter.hpp"
#include "../includes/HttpServer.hpp"
#include "../includes/Reactor.hpp"
#include "../includes/Router.hpp"
#include "../includes/RequestDispatcher.hpp"
#include "../includes/CgiHandler.hpp"
#include "../includes/Errors.hpp"

int main(int ac, char **av, char **envp)
{
    if (ac != 2) {
        Error::logs("Usage: " + std::string(av[0]) + " <config_file>");
        return 1;
    }

    try
    {
        //!=== Load & parse config === (youness should check and organize this part fo the code.)
        ConfigInterpreter parser;
        parser.getConfigData(av[1]);
        parser.parse();
        parser.checkValues();
        std::string cgiEnv = parser.getPathForCGI(envp);

        std::cout << "[✔] Config loaded. CGI path: " << cgiEnv << std::endl;

        std::vector<ServerConfig> configs = parser.getServerConfigs();
        std::vector<HttpServer*> servers;

        //* === Setup servers === (done 100%)
        for (size_t i = 0; i < configs.size(); ++i) {
            HttpServer* server = new HttpServer(configs[i]);
            server->setup();
            servers.push_back(server);
        }

        //* === Setup the multiplixing monitor aka reactor === (done 99.99%)
        Reactor reactor;
        for (size_t i = 0; i < servers.size(); ++i)
            reactor.registerServer(*servers[i]);

        //* === Event loop ===
        size_t MaxSizeToUPload;
        while (true)
        {
            reactor.poll(); //? hna kan3mer wa7d struct smitha Event ghatl9awha f reactor class
            std::vector<Event> events = reactor.getReadyEvents(); //? Hna kangeti dik struct li fiha evensts li 3mrathom poll (kernel li 3merhom poll it's system call you for more infos go to reactor.cpp > void Reactor::poll())

            //? hna kanlopi ela ga3 events struct kola wahd o chno khasni ndir lih/bih isnewconnection isReadble (POLLIN) isWritble
            for (size_t i = 0; i < events.size(); ++i)
            {
                Event event = events[i];

                if (event.isNewConnection)
                {
                    HttpServer* server = reactor.getServerByListeningFd(event.fd); //? hna kanchof ina server t connecta m3ah l client bach nchof ina route khsni nmchi lih mn ber3d
                    MaxSizeToUPload = server->getConfig().clientMaxBodySize;
                    std::cout << "==========================MAX-body-Size=====================" << std::endl;
                    std::cout << "Max body size for this server: " << MaxSizeToUPload << " bytes" << std::endl;
                    std::cout << "==========================/MAX-body-Size=====================" << std::endl;

                    if (server)
                    {
                        Connection* conn = new Connection(server->acceptConnection(event.fd)); //? hna kan 9ad wahd object dial connection kan constructih b client object li kay creah (acceptih) server
                        reactor.addConnection(conn, server);
                        std::cout << NEW_CLIENT_CON << std::endl;
                    }
                }
                else if (event.isReadable)
                {
                    Connection& conn = reactor.getConnection(event.fd); //@ kangeti dak l connection li tcreat (katkon fmap)
                    std::string data = conn.readData(); //@ (request li kaysifto l kliyan)This reads new data and accumulates in buffer
                    std::cout << data << std::endl; //TODO remove this line later on no need for it
                    //@ Check if we have complete headers hna fin kantsna ywselni request camel
                    size_t headerEnd = conn.getBuffer().find("\r\n\r\n");
                    if (headerEnd == std::string::npos) {
                        Error::logs(INCOMPLATE_HEADER); //! maybe i'll remove it later on in here and in httpserver.hpp
                        continue;
                    }

                    //@ Headers are complete, now check if body is complete (for POST requests)
                    std::string headerPart = conn.getBuffer().substr(0, headerEnd + 4);
                    std::string remainingData = conn.getBuffer().substr(headerEnd + 4);

                    size_t contentLength = 0;
                    size_t clPos = headerPart.find("content-length:");
                    if (clPos != std::string::npos) {
                        size_t clStart = headerPart.find(":", clPos) + 1;
                        size_t clEnd = headerPart.find("\r\n", clStart);
                        if (clEnd != std::string::npos)
                        {
                            std::string clStr = headerPart.substr(clStart, clEnd - clStart);
                            clStr.erase(0, clStr.find_first_not_of(" \t"));
                            clStr.erase(clStr.find_last_not_of(" \t") + 1);
                            contentLength = std::stoul(clStr);
                        }
                    }
                    if (contentLength > MaxSizeToUPload)
                    {
                        HttpServer* server = reactor.getServerForClient(event.fd);
                        Error::logs("Request body exceeds maximum size limit.");
                        HttpResponse resp;
                        resp.version = "HTTP/1.1";
                        resp.statusCode = 413;
                        resp.statusText = "Payload Too Large";
                        resp.body = Error::loadErrorPage(413, server->getConfig());
                        resp.headers["content-length"] = std::to_string(resp.body.size());
                        resp.headers["content-type"] = "text/html";
                        conn.writeData(resp.toString());
                        reactor.removeConnection(event.fd);
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
                                std::cout << "[Response status]: " << resp.statusCode << std::endl;
                                std::cout << "[Response body]: " << resp.body << std::endl;
    
                            }
                        } else {
                            resp.statusCode = 404;
                            resp.statusText = "Not Found";
                            resp.body = Error::loadErrorPage(404, server->getConfig());
                            resp.headers["content-length"] = std::to_string(resp.body.size());
                        }

                        conn.writeData(resp.toString());
                        reactor.removeConnection(event.fd); //! later i shouldn't remove this !!
                        std::cout << "\033[1;31m[-]\033[0m Connection closed" << std::endl;
                    }
                    catch (const std::runtime_error& e) {
                        std::string msg = e.what();
                        if (msg.find("incomplete body") != std::string::npos) {
                            continue;
                        } else {
                            std::cerr << "Parse error: " << msg << std::endl;
                            HttpResponse errorResp;
                            errorResp.statusCode = 400;
                            errorResp.statusText = "Bad Request";
                            errorResp.body = "HTTP/1.1 400 Bad Requestooooo\r\n\r\nBad Request";
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
