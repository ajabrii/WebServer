/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   old_webserv.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: baouragh <baouragh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: Invalid date        by                   #+#    #+#             */
/*   Updated: 2025/04/29 14:53:58 by baouragh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


# ifndef OLD_WEBSERV_HPP
#define OLD_WEBSERV_HPP

# include <iostream>
# include <fstream>
# include <string>
#include <cstdlib>
# include <vector>
# include <cctype>
# include <ctime>
# include <map>
#include <cstring>      // For memset
#include <unistd.h>     // For close()
#include <sys/socket.h> // For socket functions
#include <netinet/in.h> // For sockaddr_in
#include <arpa/inet.h>  // For inet_ntoa()


#define RED "\033[1;31m"
#define GREEN "\033[1;32m"
#define YELLOW "\033[1;33m"
#define RES "\033[0m"



typedef struct s_routes_block
{
    std::string location;
    std::string methods;
} t_route_block;

typedef struct _server_block
{
    int index;
    std::string server_name;
    std::string listen; // port
    std::string error_page; // 404.html
    std::string client_max_body_size;
    std::string host;
    std::vector<t_route_block> routes;
    
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
        t_server_block getBlock(int index);
        void printBlock(t_server_block block);
        int serverFlag;
        int routeFlag;
        
};

# endif