/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/25 16:39:13 by ajabri            #+#    #+#             */
/*   Updated: 2025/04/25 17:10:05 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# ifndef WEBSERV_HPP
#define WEBSERV_HPP

# include <iostream>
# include <fstream>
# include <string>
#include <cstdlib>

class WebServ
{
    private:
        std::string m_FileName;
    public:
        WebServ();
        WebServ(std::string config);
        WebServ(const WebServ& other);
        WebServ& operator=(const WebServ& other);
        ~WebServ();
        std::string GetFileName();
        void OpenConfigFile(std::fstream& configFile);
        static void ServerLogs(std::string logs);
};

# endif