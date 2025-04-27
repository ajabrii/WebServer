/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: baouragh <baouragh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/16 18:44:48 by kali              #+#    #+#             */
/*   Updated: 2025/04/27 19:06:36 by baouragh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "webserv.hpp"

//? youness read .config file and parse it

int main(int ac, char **av)
{
    // clock_t start, end;

    // start = clock();
    WebServ data(av[1]);
    
    (void)av;
    if (ac != 2) {
       WebServ::ServerLogs("Error:\n[usage]-> ./webserv configFile.config.");
        return (1);
    }
    std::fstream configFile;
    data.ReadConfig(configFile);

    data.printBlock(data.getBlock(0));

    Socket s(AF_INET, SOCK_STREAM, 0);

    Server serv(s);

    std::cout <<  serv.getSocket().getFd() << std::endl;
    
    
    // end = clock();
    // double time_taken = double(end - start) / double(CLOCKS_PER_SEC);   
    // std::cout << "Time taken by function: " << time_taken << " seconds" << std::endl;
    return (0);
}

//* read the file content