#include "Webserver.hpp"

WebServ::WebServ()
{
    m_FileName = "./config/default_config.config";
}

WebServ::WebServ(std::string config) : m_FileName(config), serverFlag(0), routeFlag(0)
{
    std::string line;
    size_t commentPos;
    std::vector<std::string> blockKeywords = {"server", "location", "route"};
    bool matched;
    std::string keyword;
    // std::cout << m_FileName << "\n";
    std::ifstream infile(config.c_str());
    if (!infile.is_open())
        throw std::runtime_error("Cannot open config file");
    if (infile.peek() == std::ifstream::traits_type::eof())
        throw std::runtime_error("Config file is empty");
    
    while (std::getline(infile, line))
    {
        commentPos = line.find('#'); //?  here i handle the position of the comment like "server { #comment"
        if (commentPos != std::string::npos)
            line = line.substr(0, commentPos);

        line = trim(line);
        if (line.empty())
            continue;
        matched = false;
        for (size_t i = 0; i < blockKeywords.size(); ++i) {
            keyword = blockKeywords[i];

            if (line == keyword) {
                m_ConfigData.push_back(keyword);
                matched = true;
                break;
            }

            //? handell this problem :: "keyword {" or "keyword  \n {"
            if (line.find(keyword) != std::string::npos && line.find("{") != std::string::npos) {
                size_t keyPos = line.find(keyword);
                size_t bracePos = line.find("{");

                if (keyPos < bracePos) {
                    m_ConfigData.push_back(trim(line.substr(0, bracePos)));
                    m_ConfigData.push_back("{");
                    matched = true;
                    break;
                }
            }
        }

        if (!matched) {
            m_ConfigData.push_back(line);
        }
    }
    // for (size_t i = 0; i < m_ConfigData.size(); ++i)
    // {
    //     std::cout << "std::line :: " << m_ConfigData[i] << "\n";
    // }
    infile.close();
}

std::vector<Server_block> WebServ::getServerBlocks() const
{
    return m_ServerBlocks;
}

WebServ::WebServ(const WebServ& other)
{
    *this = other; //TODO
}

WebServ& WebServ::operator=(const WebServ& other)
{
    //todo
   (void)other;
    return *this;
}

WebServ::~WebServ()
{
}

bool WebServ::Isspaces(const std::string& line) 
{
    for (size_t i = 0; i < line.length(); i++)
    {
        if (!std::isspace(line[i]))
            return false;
    }
    return true;
}

bool WebServ:: IsComment(const std::string& line) 
{
    //! handle comment in the same line ex: error_page 404 = /404.html; #comment 
    //? done
    if (line.empty() || line.find('#') != std::string::npos || Isspaces(line))
        return true;
    return false;
}

void WebServ::parseConfig()
{
    Server_block current_server;
    RouteConfig current_route;
    std::string line;

    // for (size_t i = 0; i < m_ConfigData.size(); i++)
    // {
    //     std::cout << "std::line :: " << m_ConfigData[i] << "\n";
    // }
    for (size_t i = 0; i < m_ConfigData.size(); i++)
    {
        line = m_ConfigData[i];
        if (IsComment(line)) 
            continue;

        if (line == "server")
        {
            if (i + 1 < m_ConfigData.size() && m_ConfigData[i + 1] == "{")
            {
                serverFlag = 1;
                ++i;
                continue;
            }
            throw std::runtime_error("Invalid server block syntax");
        }

        if (line == "}")
        {
            if (routeFlag)
            {
                current_server.routes.push_back(current_route);
                current_route = RouteConfig();
                routeFlag = 0;
            }
            else if (serverFlag)
            {
                m_ServerBlocks.push_back(current_server);
                current_server = Server_block();
                serverFlag = 0;
            }
            else
            {
                throw std::runtime_error("Invalid config file syntax: unexpected closing brace '}' without a matching opening block");
            }
            continue;
        }

        if (line == "{")
        {
            if (!serverFlag && !routeFlag)
                throw std::runtime_error("Invalid config file syntax: unexpected '{' without a preceding block keyword");
        }

        if (line.compare(0, 5, "route") == 0 || line.compare(0, 8, "location") == 0)
        {
            if (i + 1 < m_ConfigData.size() && m_ConfigData[i + 1] == "{")
            {
                current_route.path = extractPathFromRouteLine(line);
                routeFlag = 1;
                ++i;
                continue;
            }
            throw std::runtime_error("Invalid route block syntax");
        }

        if (serverFlag)
        {
            if (routeFlag)
            {
                parseRouteLine(current_route, line);
            }
            else
            {
                parseServerLine(current_server, line);
            }
        }
    }
}

//* run the entry point of the web server !!
void WebServ::run()
{
    this->parseConfig();
    // printConfig();
    // checkValues();
    this->IniServers();
    std::cout << GREEN"Servers running..." << RES << std::endl;
    this->addPollFDs();
    this->eventLoop();
}

int WebServ::set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void WebServ::IniServers()
{
    size_t size = this->getServerBlocks().size();
    // Socket socks[size];
    std::vector<Socket> socks(size);

    for (size_t i = 0; i < size; ++i)
    {
        //*1 here we create the socket
        socks[i].setSocket(AF_INET, SOCK_STREAM, 0);
        Server server(socks[i]);

        // std::cout << "Socket created with fd: " << server.getSocket().getFd() << std::endl;

        sockaddr_in addr;
        std::memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(this->getServerBlocks()[i].port);
        // std::cout << data.getServerBlocks()[i].port << std::endl;
        addr.sin_addr.s_addr = INADDR_ANY;
        
        //*2 here we create the address and bind it to the socket
        if (bind(socks[i].getFd(), (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            perror("bind");
            close(socks[i].getFd());
            continue;
        }
        //*3 here we listen to the socket
        if (listen(socks[i].getFd(), CLIENT_QUEUE) < 0) {
            perror("listen");
            close(socks[i].getFd());
            continue;
        }
        //*4 here we set the socket to non-blocking
        if (set_nonblocking(socks[i].getFd()) < 0) {
            perror("fcntl");
            close(socks[i].getFd());
            continue;
        }

        // std::cout << "Server listening on port " << data.getServerBlocks()[i].port << std::endl;
        server.setServerAddress(addr); // implement this if not already
        //*5 storing the server block to the server vector 
        this->m_Servers.push_back(server);
    }
}



void WebServ::addPollFDs()
{
    pollfd pfd;
    // std::vector<Server> servers = data.getServers();
    for (size_t i = 0; i < this->m_Servers.size(); ++i) {
        pfd.fd = this->m_Servers[i].getSocket().getFd();
        this->m_isServFD[pfd.fd] = true;
        pfd.events = POLLIN;
        pfd.revents = 0;
        this->m_PollFDs.push_back(pfd);
    }

}

void WebServ::eventLoop()
{
    // std::map<int, Client> clients;
    while (true) {
        // std::cout << "Waiting for events..." << std::endl;
        int ret = poll(this->m_PollFDs.data(), this->m_PollFDs.size(), -1);
        if (ret < 0) {
            perror("poll");
            break;
        }

        for (size_t i = 0; i < this->m_PollFDs.size(); ++i)
        {
            // std::cout << "Waiting for events..." << std::endl;
            if (this->m_PollFDs[i].revents & POLLIN) {

                if (this->m_isServFD[this->m_PollFDs[i].fd] == true)
                {
                    
                    sockaddr_in client_addr;
                    socklen_t addr_len = sizeof(client_addr);
                    int client_fd = accept(this->m_PollFDs[i].fd, (struct sockaddr*)&client_addr, &addr_len);
                    if (client_fd < 0) {
                        std::cerr << "Error accepting connection  " << client_fd<<std::endl;
                        // perror("accept");
                        continue;
                    } else {
                        if (set_nonblocking(client_fd) < 0) {
                            perror("fcntl");
                            close(client_fd);
                            continue;
                        }
                        this->m_Clients[client_fd] = Client(client_fd, client_addr, addr_len);
                        pollfd clintpfd;
                        clintpfd.fd = client_fd;
                        clintpfd.events = POLLIN;
                        clintpfd.revents = 0;
                        this->m_PollFDs.push_back(clintpfd);
                        std::cout << "New client connected: " << inet_ntoa(client_addr.sin_addr) << ":" << ntohs(client_addr.sin_port) << std::endl;
                    }
                }
            // *1. receive the request
                } else if (this->m_isServFD[this->m_PollFDs[i].fd] == false) {
                    // here i will receive the request
                    int client_fd = this->m_PollFDs[i].fd;
                    char buffer[1024];
                    ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
                    if (bytes_received > 0) {
                        buffer[bytes_received] = '\0'; // Null-terminate the received data
                        std::cout << "Received request:\n" << buffer << std::endl;
                        
                        // *2. send the response
                        std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!";
                        send(client_fd, response.c_str(), response.size(), 0);
                        
                    } else if (bytes_received < 0) {
                            // !~remove the client from the this->m_PollFDs and clients map (disconnect ones)
                            std::cerr << "Error receiving data from client: " << client_fd << std::endl;
                            close(client_fd);
                            this->m_PollFDs.erase(this->m_PollFDs.begin() + i);
                            this->m_Clients.erase(client_fd);
                            this->m_isServFD.erase(client_fd);
                            i--;
                        perror("recv");
                    }
                }
            }
        }
}