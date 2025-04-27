/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/25 16:39:13 by ajabri            #+#    #+#             */
/*   Updated: 2025/04/27 16:33:52 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# ifndef WEBSERV_HPP
#define WEBSERV_HPP

# include <iostream>
# include <fstream>
# include <string>
#include <cstdlib>
# include <vector>
# include <cctype>
# include <ctime>
# include <map>

typedef struct s_routes_block
{
    std::string location;
    std::string methods;
} t_route_block;

typedef struct _server_block
{
    std::string server_name;    // this is for server name
    std::string listen; // this is for listen
    std::string error_page; // this is for error page
    std::string client_max_body_size; // this is for max body size
    std::string host;
    // std::string root;   // this is for root
    // std::string autoindex; // this is for autoindex
    // std::string upload; // this is for upload
    // std::string cgi_pass; // this is for cgi 
    std::vector<t_route_block> routes; // this is for routes
    
} t_server_block;


class WebServ
{
    private:
        std::string m_FileName;
        std::vector<std::string> m_ConfigData;
        std::vector<t_server_block> m_ServerBlocks;

    public:
        WebServ();
        WebServ(std::string config);
        WebServ(const WebServ& other);
        WebServ& operator=(const WebServ& other);
        ~WebServ();
        std::string GetFileName();
        void OpenConfigFile(std::fstream& configFile);
        void ReadConfig(std::fstream& configFile);
        std::vector<std::string> GetConfigData();
        static void ServerLogs(std::string logs);
        void dataScraper(std::vector<std::string> &lines);
        void ServerData(std::vector<std::string> &lines, size_t& i);
        std::string Trim(std::string& str);
        void getServerData(std::string& line, t_server_block& block);
        void getRouteData(std::string& line, t_server_block& block);
        int serverFlag;
        int routeFlag;
        
};

# endif