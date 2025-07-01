/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ytarhoua <ytarhoua@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 15:39:15 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/01 15:47:53 by ytarhoua         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "../includes/ConfigInterpreter.hpp"
# include "../includes/HttpServer.hpp"
# include "../includes/Reactor.hpp"
# include "../includes/Router.hpp"
# include "../includes/RequestDispatcher.hpp"


int main(int ac, char **av)
{
    if (ac != 2) {
        std::cerr << "error: need config file !!\n";
        return 1;
    }
    try
    {
        ConfigInterpreter parser;
        parser.getConfigData(av[1]);
        parser.parse();
        parser.checkValues();
        std::cout << "everything is good " << std::endl;
        // exit(0);
        std::vector<ServerConfig> configs = parser.getServerConfigs();

        std::vector<HttpServer*> servers;

        // Create server for each config
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
            reactor.poll();  //? Wait for events
            std::vector<Event> readyEvents = reactor.getReadyEvents();

            for (size_t i = 0; i < readyEvents.size(); ++i)
            {
                Event event = readyEvents[i];

                if (event.isNewConnection) {
                    HttpServer* server = reactor.getServerByListeningFd(event.fd);
                    if (server) {
                        Connection* conn = new Connection(server->acceptConnection());
                        reactor.addConnection(conn, server);
                        std::cout << "[+] New client connected" << std::endl;
                    }
                }
                else if (event.isReadable) {
                    
                    Connection& conn = reactor.getConnection(event.fd);
                    std::string data = conn.readData();
                    // std::cout << "\033[1;32m" << data << "\033[0m" << std::endl;

                    if (!data.empty()) {
                        HttpRequest req = HttpRequest::parse(data);
                        
                         Router router;
                        // std::cout <<"serverMap size: " <<reactor.serverFd << std::endl;
                         HttpServer* server = reactor.getServerForClient(event.fd);
                        // if (!server) {
                        //     std::cerr << "No server found for fd: " << event.fd << std::endl;
                        //     continue;
                        // }
                        
                         std::cout << server->getConfig().serverName[1] << std::endl;
                        
                         const RouteConfig* route = router.match(req, server->getConfig());
                         std::cout << "route ------------->>>> " << route->path << "\n";
                         std::cout << "paaath -------------->>>" << route->root << "\n";
                         HttpResponse response;
                         if (route) {
                            //* check for cgi
                             // Dispatch request to the matched route
                             RequestDispatcher dispatcher;
                             response = dispatcher.dispatch(req, *route);
                             std::cout << "[<] Response status: " << response.statusCode << " " << response.statusText << std::endl;
                                std::cout << "[<] Response body: " << response.body << std::endl;
                         } else {  
                             // Handle 404 Not Found
                                std::cout << "No matching route found for request" << std::endl;
                             response.statusCode = 404;
                             response.statusText = "Not Found";
                             response.body = "The requested resource was not found.";
                         }
                         std::string responseStr = response.toString();
                        conn.writeData(responseStr);
                        // if (!servers.empty()) {
                        //     HttpResponse res = servers[0]->handleRequest(req);
                        //     // RequestDispatcher dispatcher;
                        //     std::string responseStr = res.toString();
                        //     conn.writeData(responseStr);
                        //     std::cout << "[<] Sent response of " << responseStr.size() << " bytes" << std::endl;
                        // }

                        // ✅ remove after responding
                        reactor.removeConnection(event.fd);
                        std::cout << "[-] Closed connection after response" << std::endl;
                    }
                }
            }
        }

        // Cleanup before exit (unreachable here)
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