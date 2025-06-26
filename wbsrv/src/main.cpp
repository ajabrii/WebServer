/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 15:39:15 by ajabri            #+#    #+#             */
/*   Updated: 2025/06/26 17:17:47 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "../includes/ConfigInterpreter.hpp"
# include "../includes/HttpServer.hpp"

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
        

    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
}


// for (size_t i = 0; i < configs.size(); i++)
        // {
        //     std::cout <<"max bodysize "<< configs[i].clientMaxBodySize << "\nhost " << configs[i].host << std::endl;
        //     std::cout << "port " << configs[i].port << "\nerrorpage " << configs[i].errorPage << std::endl;
        //     std::cout << "servername " << configs[i].serverName << std::endl;  
        //     for (size_t x = 0; x < configs[i].routes.size(); x++)
        //     {
        //         // std::cout << "\n";
        //         std::cout << configs[i].routes[x].allowedMethods[0] << "\n";
        //         std::cout << configs[i].routes[x].path << "\n";
        //         std::cout << configs[i].routes[x].root << "\n";
        //         std::cout << configs[i].routes[x].redirection << "\n";
        //         std::cout << configs[i].routes[x].autoindex << "\n";
        //         std::cout << configs[i].routes[x].indexFile << "\n";
        //         std::cout << configs[i].routes[x].cgiExtension << "\n";
        //         std::cout << configs[i].routes[x].uploadDir<< "\n";                
        //     }          
        // }