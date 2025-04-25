/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/16 18:44:48 by kali              #+#    #+#             */
/*   Updated: 2025/04/25 17:27:49 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "webserv.hpp"

//? youness read .config file and parse it

int main(int ac, char **av)
{
    WebServ data(av[1]);
    
    (void)av;
    if (ac != 2) {
       WebServ::ServerLogs("Error:\n[usage]-> ./webserv configFile.config.");
        return (1);
    }
    std::fstream configFile;
    data.ReadConfig(configFile);
    
    
    return (0);
}

//* read the file content