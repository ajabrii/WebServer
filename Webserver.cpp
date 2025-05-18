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

//! this function is for printing error in std::ceer;
void WebServ::logs(std::string err)
{
    std::cerr << err << std::endl;
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
    this->Login();

    this->IniServers();
    std::cout << GREEN"Servers running..." << RES << std::endl;
    this->addPollFDs();
    this->eventLoop();
}

int WebServ::set_nonblocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void WebServ::SoketBind(Server &serv, Socket& sock, int i)
{
    // std::cout << "Socket created with fd: " << server.getSocket().getFd() << std::endl;
    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(this->getServerBlocks()[i].port);
    addr.sin_addr.s_addr = INADDR_ANY;

    //*2 here we create the address and bind it to the socket
    if (bind(sock.getFd(), (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(sock.getFd());
        return;
    }
    serv.setServerAddress(addr); // implement this if not already
}

void WebServ::handleNewConnection(int fd)
{
    sockaddr_in client_addr = m_Clients[fd].getClientAddr();
    socklen_t addr_len = sizeof(client_addr);
    int client_fd = accept(fd, (struct sockaddr*)&client_addr, &addr_len);
    if (client_fd < 0) {
        std::cerr << "Error accepting connection  " << client_fd<<std::endl;
        // perror("accept");
        return;
    } else {
        if (set_nonblocking(client_fd) < 0) {
            perror("fcntl");
            close(client_fd);
            return;
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


void WebServ::SendError(int fd, int errorCode, std::string err)
{
    std::cerr << "Error: " << err << std::endl;
    std::string error_response = "HTTP/1.1 " + std::to_string(errorCode) + err +" \r\n";
    error_response += "Content-Type: text/html\r\n";
    error_response += "\r\n";
    error_response += "<html><body><h1>" + std::to_string(errorCode) + err +"</h1></body></html>";
    send(fd, error_response.c_str(), error_response.size(), 0);
}

void WebServ::cleanupClientFD(int fd)
{
    close(fd);
    m_Clients.erase(fd);
    m_isServFD.erase(fd);
    this->m_Requests.erase(fd);
    for (size_t i = 0; i < m_PollFDs.size(); ++i) {
        if (m_PollFDs[i].fd == fd) {
            m_PollFDs.erase(m_PollFDs.begin() + i);
            break;
        }
    }
    std::cout << "Client disconnected: " << fd << std::endl;
}

void WebServ::IniServers() 
{
    size_t size = this->getServerBlocks().size();
    std::set<uint16_t> bound_ports;
    std::vector<Socket> socks(size);

    for (size_t i = 0; i < size; ++i) {
        uint16_t port = this->getServerBlocks()[i].port;

        if (bound_ports.count(port)) {
            std::cout << YELLOW"Port " << port << " already in use, skipping duplicate bind.\n" << RES;
            continue;
        }

        bound_ports.insert(port);
        socks[i].setSocket(AF_INET, SOCK_STREAM, 0);

        int opt = 1;
        setsockopt(socks[i].getFd(), SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));// to understand this
        Server server(socks[i]);
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = INADDR_ANY;

        if (bind(socks[i].getFd(), (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            perror("bind");
            close(socks[i].getFd());
            continue;
        }

        server.setServerAddress(addr);

        if (listen(socks[i].getFd(), CLIENT_QUEUE) < 0) { // to see
            perror("listen");
            close(socks[i].getFd());
            continue;
        }
        if (set_nonblocking(socks[i].getFd()) < 0) {
            perror("fcntl");
            close(socks[i].getFd());
            continue;
        }

        this->m_Servers.push_back(server);
    }
}



void WebServ::addPollFDs()
{
    pollfd pfd;
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
    while (true) {
    
      int ret = poll(m_PollFDs.data(), m_PollFDs.size(), -1);
      if (ret < 0) { perror("poll"); break; }
  
      for (size_t i = 0; i < m_PollFDs.size(); ++i)
      {
        int fd = m_PollFDs[i].fd;
        if (m_PollFDs[i].revents & POLLIN) {
          if (m_isServFD[fd]) {
            // New connection on a listening socket
            handleNewConnection(fd);
          }
          else {
            // Data ready on a client socket
            request(fd);
          }
        }
      }
    }
  }
  

  void WebServ::request(int fd) 
  {
    Request& req = m_Requests[fd]; 

    std::cout <<RED<< req.requesto <<RES<< std::endl;
    std::cout <<RED<< req.reveivedBytes << RES<<std::endl;
    // read into the buffer at its current end
    ssize_t n = recv(fd, req.requesto + req.reveivedBytes, BUFFER_SIZE - req.reveivedBytes - 1, 0);

    if (n == 0) {
        // client closed
        req.reveivedBytes = 0;
        cleanupClientFD(fd);
        return;
    }
    if (n < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return; // no data right now
        cleanupClientFD(fd);
        return;
    }

    req.reveivedBytes += n;
    req.requesto[req.reveivedBytes] = '\0';

    // have we got the end of headers?
    if (!req.isComplate && strstr(req.requesto, "\r\n\r\n")) {
        req.isComplate = true;
        req.parseHttpRequest();
        this->m_Requests[fd] = req;
        if (req.method == GET)
        {
            this->routing(fd, req);
            std::cout << "GET request received" << std::endl;
        }
        else if (req.method == POST)
        {
            std::cout << "POST request received" << std::endl;
        }
        else if (req.method == DELETE) {
            std::cout << "DELETE request received" << std::endl;
        }
        else
        {
            std::cout << "Unknown request method: " << req.method << std::endl;
        }
    }
}


// void WebServ::request(int fd)
// {
//     Request tmp = this->m_Requests[fd];

//     ssize_t bytes_received = recv(fd,tmp.requesto, sizeof(tmp.requesto) - 1, 0);
//     if (bytes_received == 0) {
//         cleanupClientFD(fd);
//         return;
//     } else if (bytes_received < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
//         cleanupClientFD(fd);
//     }
//     if (bytes_received > 0) {
//         tmp.isComplate = false;
//         tmp.requesto[bytes_received] = '\0'; // Null-terminate the received data
//         std::cout << "Received request:\n" << tmp.requesto << std::endl;
//         tmp.parseHttpRequest();
//         this->m_Requests[fd] = tmp;
//         if (tmp.method == GET)
//         {
//             this->routing(fd, tmp);
//             std::cout << "GET request received" << std::endl;
//         }
//         else if (tmp.method == POST)
//         {
//             //handle post request
//             // this->routing(fd, tmp);
//             std::cout << "POST request received" << std::endl;
//         }
//         else if (tmp.method == DELETE) {
//             std::cout << "DELETE request received" << std::endl;
//         }
//         else
//         {
//             std::cout << "Unknown request method: " << tmp.method << std::endl;
//         }
//     }
// }

std::string WebServ::clean_line(std::string line)
{
    // Trim leading and trailing whitespace
    line.erase(0, line.find_first_not_of(" \t\n\r"));
    line.erase(line.find_last_not_of(" \t\n\r") + 1);

    // Remove trailing semicolon if it exists
    if (!line.empty() && line.back() == ';')
    {
        line.pop_back();
    }
    return line;
}

void WebServ::routing(int fd, Request& request)
{
    //! Here I will route the request to the right server but i have to find the server first
    std::cout << "Routing request to server..." << std::endl;
    bool serverFound = false;

    for (size_t i = 0; i < this->m_Servers.size(); ++i)
    {
        std::string requestHost = clean_line(request.headers["Address"]);
        std::string serverHost = clean_line(this->m_ServerBlocks[i].host);

        std::cout << "----------------dkhl ---------------: " << fd << std::endl;
        std::cout << "requestHost: " << requestHost << std::endl;
        std::cout << "serverHost: " << serverHost << std::endl;
        // Treat "localhost" and "0.0.0.0" as equivalent
        if ((requestHost == "localhost" && serverHost == "0.0.0.0") || 
            (requestHost == "0.0.0.0" && serverHost == "localhost") || 
            (requestHost == serverHost))
        {
            if (this->m_ServerBlocks[i].port == std::atoi(request.headers["Port"].c_str()))
            {
                // Server found, handle the request
                std::cout << "Server found for fd: " << fd << std::endl;
                // fd = this->m_Servers[i].getSocket().getFd();
                serverFound = true;
                this->matchingRoute(request, i, fd);
                break;
            }
        }
    }

    // If no matching server was found, send an error response
    if (!serverFound)
    {
        std::cerr << "Error: No matching server found for fd: " << fd << std::endl;
        std::string error_response = "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/html\r\n\r\n";
        error_response += "<html><body><h1>500 Internal Server Error</h1><p>No matching server found.</p></body></html>";
        send(fd, error_response.c_str(), error_response.size(), 0);
        close(fd);
    }
}

std::string DirectoryListing(const std::string& path, const std::string& urlPath) {
    DIR* dir = opendir(path.c_str());
    if (!dir) {
        return "<html><body><h1>403 Forbidden</h1></body></html>"; // or use your error page
    }

    std::ostringstream html;
    html << "<html><head><title>Index of " << urlPath << "</title></head><body>";
    html << "<h1>Index of " << urlPath << "</h1><ul>";

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        std::string name = entry->d_name;
        if (name == ".") continue;
        std::string link = urlPath + (urlPath.back() == '/' ? "" : "/") + name;
        html << "<li><a href=\"" << link << "\">" << name << "</a></li>";
    }

    html << "</ul></body></html>";
    closedir(dir);

    return html.str();
}
void WebServ::matchingRoute(Request& request, int i, int fd)
{
    std::cout << "Matching route..." << std::endl;
    RouteConfig* match = nullptr;
    RouteConfig* defaultRoute =  nullptr;
    
    for (size_t j = 0; j < m_ServerBlocks[i].routes.size(); ++j) {
        const std::string& routePath = m_ServerBlocks[i].routes[j].path;
    
        // Save the shortest route as fallback (often "/")
        if (!defaultRoute || routePath.length() < defaultRoute->path.length()) {
            defaultRoute = &m_ServerBlocks[i].routes[j];
        }
    
        // Check if routePath is a prefix of request.path
        if (request.path.find(routePath) == 0) {
            // Longest prefix match
            if (!match || routePath.length() > match->path.length()) {
                match = &m_ServerBlocks[i].routes[j];
            }
        }
    }
    
    // Use longest matching prefix or fallback to shortest route
    if (!match) {
            match = defaultRoute;
    }
        
        // Handle redirect if any
        std::cout<< "redirect: ------------------------------------------------->>" << match->redirect << std::endl;  
        if (!match->redirect.empty()) {
            std::string redirect_response = "HTTP/1.1 301 Moved Permanently\r\nLocation: " + match->redirect + "\r\n\r\n";
            logs(redirect_response);
            send(fd, redirect_response.c_str(), redirect_response.size(), 0);
            return;
        }
        std::cout << "-----------match: " << match->path << std::endl;

        if (request.path == "/") {
            request.path = ""; // Handle root path
        }
    std::string filePath = clean_line(match->root) + clean_line(request.path);
    std::cout << "request path: " << request.path << std::endl;
    std::cout << "match root: " << match->root << std::endl;
    std::cout << "File path: " << filePath << std::endl;
    std::cout << "match->directory_listing: " << match->directory_listing << std::endl;

    if (match->directory_listing) {
        // std::string listing = DirectoryListing(filePath, request.path);
        // std::string response = "HTTP/1.1 403 Forbidden \r\nContent-Type: text/html\r\n\r\n";
        // response += listing;
        // send(fd, response.c_str(),  response.size(), 0);
        SendError(fd, 403, " Forbidden");
    } else {
        std::ifstream file(filePath.c_str());
        if (file) {
            std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
            response += content;
            send(fd, response.c_str(), response.size(), 0);
        } else {
            // std::cout << "File found: " << filePath << std::endl;
            std::cerr << "File not found: " << filePath << std::endl;
            SendError(fd, 404," not found");
            // std::string error_response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n";
            // error_response += "<html><body><h1>404 Not Found</h1></body></html>";
            // send(fd, error_response.c_str(), error_response.size(), 0);
        }
    }
}


// void WebServ::ParseRequest()
// {

// }

void WebServ::Login() {
    std::cout<<YELLOW << R"(
 _    _      _     _____
| |  | |    | |   /  ___|
| |  | | ___| |__ \ `--.  ___ _ ____   __
| |/\| |/ _ \ '_ \ `--. \/ _ \ '__\ \ / /
\  /\  /  __/ |_) /\__/ /  __/ |   \ V /
 \/  \/ \___|_.__/\____/ \___|_|    \_/
)" << RES<<std::endl;
}

