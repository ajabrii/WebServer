/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 15:39:15 by ajabri            #+#    #+#             */
/*   Updated: 2025/06/30 06:15:01 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "../includes/ConfigInterpreter.hpp"
# include "../includes/HttpServer.hpp"
# include "../includes/Reactor.hpp"


int main(int ac, char **av)
{
    if (ac != 2) {
        std::cerr << "error: need config file !!\n";
        return 1;
    }
    try
    {
        ConfigInterpreter parser;
        parser.parse(av[1]);
        std::vector<ServerConfig> configs = parser.getServerConfigs();

        std::vector<HttpServer*> servers;

        for (size_t i = 0; i < configs.size(); ++i) {
            HttpServer* server = new HttpServer(configs[i]);
            server->setup();
            servers.push_back(server);
        }

        // Print servers to confirm
        // for (size_t i = 0; i < servers.size(); ++i) {
        //     std::cout << "Server " << i + 1 << ":\n";
        //     std::cout << "  Host: " << servers[i]->getConfig().host << "\n";
        //     std::cout << "  Port: " << servers[i]->getConfig().port << "\n";
        //     std::cout << "  Server Name: " << servers[i]->getConfig().serverName << "\n";
        //     std::cout << "  Error Page: " << servers[i]->getConfig().errorPage << "\n";
        //     std::cout << "  Client Max Body Size: " << servers[i]->getConfig().clientMaxBodySize << "\n";
        // }

        Reactor reactor;
        for (size_t i = 0; i < servers.size(); ++i) {
            reactor.registerServer(*servers[i]);
        }

        while (true) {
            reactor.poll();  // Wait for readable/writable fds
            std::vector<Event> readyEvents = reactor.getReadyEvents();

            for (size_t i = 0; i < readyEvents.size(); ++i) {
                Event event = readyEvents[i];

                if (event.isNewConnection) {
                    HttpServer* server = reactor.getServer(event.fd);
                    if (server) {
                        Connection* conn = new Connection(server->acceptConnection());
                        reactor.addConnection(conn);
                    }
                }
                else if (event.isReadable) {
                    // std::cout << "Readable event on fd: " << event.fd << std::endl;
                    // Find the connection associated with this fd
                    Connection& conn = reactor.getConnection(event.fd);
                    std::string data = conn.readData();

                    if (!data.empty()) {
                        // std::cout << "Received data: " << data << std::endl;
                        HttpRequest req = HttpRequest::parse(data);
                        // For now, just use the first server to handle request
                        if (!servers.empty()) {
                            HttpResponse res = servers[0]->handleRequest(req);
                            std::string responseStr = res.toString();
                            conn.writeData(responseStr);
                        }
                    }
                }
                else if (event.isWritable) {
                    reactor.removeConnection(event.fd);
                }
            }
        }

        // ✅ Cleanup before exit (unreachable here, but good practice)
        for (size_t i = 0; i < servers.size(); ++i) {
            delete servers[i];
        }

    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }

    return 0;
}






// int main(int ac, char **av)
// {
//     if (ac != 2){
//         std::cerr << "error: need config file !!\n";
//         return 1;
//     }
//     try
//     {
//         ConfigInterpreter parser;
//         parser.parse(av[1]);
//         std::vector<ServerConfig> configs = parser.getServerConfigs();

//         std::vector<HttpServer> servers;
//         for (size_t i = 0; i < configs.size(); ++i) {
//             HttpServer server(configs[i]);
//             server.setup();
//             servers.push_back(server);
//         }

//         Reactor reactors;
//         for (size_t i = 0;i < servers.size(); i++)
//         {
//             reactors.registerServer(servers[i]);
//         }

//         while (true)
//         {
//             reactors.poll();  // Wait for readable/writable fds

//             std::vector<Event> readyEvents = reactors.getReadyEvents();
//             for (size_t i = 0; i < readyEvents.size(); i++)
//             {
//                 Event event = readyEvents[i];

//                 if (event.isNewConnection) {
//                     // Find the appropriate server for this connection
//                     HttpServer* server = reactors.getServer(event.fd);
//                     if (server) {
//                         Connection conn = server->acceptConnection();
//                         reactors.addConnection(conn);
//                     }
//                 }
//                 else if (event.isReadable) {
//                     Connection& conn = reactors.getConnection(event.fd);
//                     std::string data = conn.readData(); // Read raw HTTP request

//                     // For now, we'll assume request is complete if we have data
//                     if (!data.empty()) {
//                         // Parse the raw data into HttpRequest
//                         HttpRequest req = HttpRequest::parse(data);

//                         // Use the first server for all connections (simple case)
//                         if (!servers.empty()) {
//                             HttpResponse res = servers[0].handleRequest(req);

//                             // Convert response to string and write it
//                             std::string responseStr = res.toString();
//                             conn.writeData(responseStr);
//                         }
//                     }
//                 }
//                 else if (event.isWritable) {
//                     // Handle writable events - for now we'll just remove finished connections
//                     // You may need to implement connection state tracking
//                     reactors.removeConnection(event.fd);
//                 }
//             }
//         }

//     }
//     catch(const std::exception& e)
//     {
//         std::cerr << e.what() << '\n';
//     }

// }