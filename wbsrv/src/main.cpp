/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 15:39:15 by ajabri            #+#    #+#             */
/*   Updated: 2025/06/26 17:48:05 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "../includes/ConfigInterpreter.hpp"
# include "../includes/HttpServer.hpp"
# include "../includes/Reactor.hpp"

int main(int ac, char **av)
{
    if (ac != 2){
        std::cerr << "error: need config file !!\n";
        return 1;
    }
    try
    {
        ConfigInterpreter parser;
        parser.parse(av[1]);
        std::vector<ServerConfig> configs = parser.getServerConfigs();
        
        std::vector<HttpServer> servers;
        for (size_t i = 0; i < configs.size(); ++i) {
            HttpServer server(configs[i]);
            server.setup();
            servers.push_back(server);
        }
        
        Reactor reactors;
        for (size_t i = 0;i < servers.size(); i++)
        {
            reactors.registerServer(servers[i]);
        }
        
        while (true)
        {
            reactors.poll();  // Wait for readable/writable fds

            std::vector<Event> readyEvents = reactors.getReadyEvents();
            for (size_t i = 0; i < readyEvents.size(); i++)
            {
                Event event = readyEvents[i];

                if (event.isNewConnection) {
                    // Find the appropriate server for this connection
                    HttpServer* server = reactors.getServer(event.fd);
                    if (server) {
                        Connection conn = server->acceptConnection();
                        reactors.addConnection(conn);
                    }
                }
                else if (event.isReadable) {
                    Connection& conn = reactors.getConnection(event.fd);
                    std::string data = conn.readData(); // Read raw HTTP request
                    
                    // For now, we'll assume request is complete if we have data
                    if (!data.empty()) {
                        // Parse the raw data into HttpRequest
                        HttpRequest req = HttpRequest::parse(data);
                        
                        // Use the first server for all connections (simple case)
                        if (!servers.empty()) {
                            HttpResponse res = servers[0].handleRequest(req);
                            
                            // Convert response to string and write it
                            std::string responseStr = res.toString();
                            conn.writeData(responseStr);
                        }
                    }
                }
                else if (event.isWritable) {
                    // Handle writable events - for now we'll just remove finished connections
                    // You may need to implement connection state tracking
                    reactors.removeConnection(event.fd);
                }
            }
        }

    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
}