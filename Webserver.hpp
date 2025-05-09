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
#include <cstring>      // For memset
#include <unistd.h>     // For close()
#include <sys/socket.h> // For socket functions
#include <netinet/in.h> // For sockaddr_in
#include <arpa/inet.h>  // For inet_ntoa()
#include <sstream> // for stringstream() to spil string
# include <poll.h>
# include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>

#include "Socket/Socket.hpp"
#include "Server/Server.hpp"
#include "Client/Client.hpp"
# include "HTTP/Request.hpp"

#define RED "\033[1;31m"
#define GREEN "\033[1;32m"
#define YELLOW "\033[1;33m"
#define RES "\033[0m"

# define CLIENT_QUEUE 0


struct RouteConfig {
    std::string path;                // Example: "/images"
    std::vector<std::string> methods; // Example: GET, POST
    std::string redirect;             // Example: "http://example.com"
    std::string root;                 // Example: "/var/www/images"
    bool directory_listing = false;   // true / false
};

struct Server_block {
    std::string host;                  // Example: "0.0.0.0"
    int port;                           // Example: 8080
    std::vector<std::string> server_names; // Example: ["mysite.com", "www.mysite.com"]
    size_t client_max_body_size = 0;    // Example: 5MB
    std::map<int, std::string> error_pages; // Example: 404 -> "/404.html"
    std::vector<RouteConfig> routes;    // List of routes

    Server_block();                    // Constructor
};

struct Config {
    std::vector<Server_block> servers;  // Multiple servers
};



class WebServ
{
    private:
        std::string m_FileName;
        std::vector<std::string> m_ConfigData;
        std::vector<Server_block> m_ServerBlocks;
        std::vector<Server> m_Servers; // server objects
        std::vector<pollfd> m_PollFDs; // pollfd objects
        std::map<int, bool> m_isServFD; // map to check if the fd is a server fd
        std::map<int, Client> m_Clients; // map to store client objects

        std::map<int, Request> m_Requests;

    public:
        WebServ();
        WebServ(std::string config);
        WebServ(const WebServ& other);
        WebServ& operator=(const WebServ& other);
        ~WebServ();

        void parseConfig();

        std::string trim(const std::string& str);
        void parseServerLine(Server_block& server, const std::string& line);
        std::string extractPathFromRouteLine(const std::string& line);
        void parseRouteLine(RouteConfig& route, const std::string& line);
        void checkValues() const;
        int serverFlag;
        int routeFlag;

        std::vector<Server_block> getServerBlocks() const;
        bool Isspaces(const std::string& line);
        bool IsComment(const std::string& line);
        void printConfig() const;

        void Login();
        int set_nonblocking(int fd);
        void run(); // this is the entry point of the web server
        //*1 create a socket and bind it to the address
        void IniServers();
        void SoketBind(Server &serv, Socket& sock, int i);
        //*2 create pollfd vector and add the server fds to it
        void addPollFDs();
        //*3 event loop
        void eventLoop();
        void handleNewConnection(int fd);

        void handleClient(int fd);
        void handleServer(int fd);
        void handleError(int fd);

        std::vector<Server> getServers() const
        {
            return m_Servers;
        }
        // std::vector<pollfd> getPollFDs() const
        // {
        //     return m_PollFDs;
        // }
        void request(int fd);
        // void ParseRequest();
        void routing(int fd, Request& req);
        void matchingRoute(int fd, Request& req);
        void buildRedirect(std::string redirect);

        static void  logs(std::string err);
};

#endif