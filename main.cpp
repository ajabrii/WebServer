/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/16 18:44:48 by kali              #+#    #+#             */
/*   Updated: 2025/04/29 10:00:38 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "webserv.hpp"
// include poll() header
# include <poll.h>

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
    std::cout << "=========================SERVER-BLOCK============================" << std::endl;

    // int sockFd;
    // int clientFd;
    // struct sockaddr_in address;
    // int addrslen = sizeof(address);

    // (void)clientFd; 
    // (void)addrslen;
    // // *1. Create socket (IPv4, TCP)
    // sockFd = socket(AF_INET, SOCK_STREAM, 0);
    // if (sockFd == -1) {
    //     WebServ::ServerLogs("Error: socket creation failed");
    //     return 1;
    // }
    // //*2. Set address info
    // memset(&address, 0, sizeof(address)); // we memset the address to 0 so we can avoid garbage values
    // address.sin_family = AF_INET; // sin_family = AF_INET means we are using IPv4
    // address.sin_addr.s_addr = INADDR_ANY; // sin_addr = INADDR_ANY means we are using all interfaces default interface
    // address.sin_port = htons(8081); // htons() stands for host to network short, it converts the port number from host byte order to network byte order
    // //*3. Bind socket to address

    // //? bind() associates the socket with the address and port number specified in the sockaddr_in structure (example: address = 192.168.43.1 and port = 8080sd )
    // if (bind(sockFd,(struct sockaddr *)&address, sizeof(address)) == -1) {
    //     WebServ::ServerLogs("Error: bind failed");
    //     close(sockFd);
    //     return 1;
    // }
    
    // //*4. Listen
    // while (1) {
        
    //     if (listen(sockFd, 5) == -1) {
    //         WebServ::ServerLogs("Error: listen failed");
    //         close(sockFd);
    //         return 1;
    //     }
    //     WebServ::ServerLogs(GREEN"Server listening on pot 8080..."RES);
    //     // ? listen accepts in it parameter the number of clients that can be queued to connect to the server. and the socket fd.
    //     //*5. Accept loop (accept one client for now)
    //     clientFd = accept(sockFd, (struct sockaddr *)&address, (socklen_t*)&addrslen);
    //     if (clientFd == -1) {
    //         WebServ::ServerLogs("Error: accept failed");
    //         close(sockFd);
    //         return 1;
    //     }
    //     std::cout <<YELLOW<< "Client connected from " <<RES<<GREEN<< inet_ntoa(address.sin_addr) <<RES <<std::endl;
    //     // ? accept() accepts a connection on the socket and creates a new socket for the client. It returns the new socket file descriptor.
    //  // Here is your new HTML body
    //     const char *html_body = 
    //     "<!DOCTYPE html>\n"
    //     "<html lang=\"en\">\n"
    //     "<head>\n"
    //     "<meta charset=\"UTF-8\">\n"
    //     "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
    //     "<title>ARPANET++</title>\n"
    //     "<link rel=\"icon\" href=\"./www/t.png\" type=\"image/x-icon\">"
    //     "</head>\n"
    //     "<body>\n"
    //     "<div style=\"display: flex; justify-content: center; align-items: center; height: 100vh; background-color: #f0f0f0;\">"
    //         "<h1 style=\"display: flex; font-weight:200; flex-direction: column;\">Webserv repo </h1>\n"
    //         "<button style=\"background-color: green; color: white; padding: 10px 20px; border: none; border-radius: 5px; margin-left: 10px\"><a href=\"https://github.com/ajabrii/WebServer/\" style=\"text-decoration: none; color: white;\">Click Me</a></button>"
    //     "</div>"
    //     "</body>\n"
    //     "</html>\n";

    //     // Create the full HTTP response
    //     char response[4096];
    //     sprintf(response,
    //     "HTTP/1.1 404 OK\r\n"
    //     "Content-Type: text/html\r\n"
    //     "Content-Length: %lu\r\n"
    //     "\r\n"
    //     "%s",
    //     strlen(html_body),
    //     html_body);

    //     // Send the HTTP response
    //     send(clientFd, response, strlen(response), 0);

        
    //     // ? send() sends the response to the client. It takes the socket file descriptor, the response string, the length of the response, and flags (0 in this case).
    //     //*7. Close sockets
    
    //     char buffer[1024];
    //     int bytesReceived = recv(clientFd, buffer, sizeof(buffer) - 1, 0);
    //     if (bytesReceived > 0) {
    //         buffer[bytesReceived] = '\0'; // Null-terminate the received data
    //         std::cout  << buffer << std::endl;
    //     } else {
    //         std::cerr << "Error receiving data" << std::endl;
    //     }
    // }
    // // while (1)
    // //     ;
    // // ? poll() is used to wait for events on file descriptors. In this case, it waits for 1 second (1000 milliseconds) before continuing.
    
    
    
    
    // // ? recv() receives data from the client. It takes the socket file descriptor, a buffer to store the received data, the size of the buffer, and flags (0 in this case).
    
    // close(clientFd);
    // close(sockFd);
    

}


// #include <iostream>
// #include <string>
// #include <cstring>
// #include <unistd.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>
// #include <sys/socket.h>

// #define PORT 8080

// std::string html_page() {
//     return 
//         "<!DOCTYPE html>\n"
//         "<html lang=\"en\">\n"
//         "<head>\n"
//         "<meta charset=\"UTF-8\">\n"
//         "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
//         "<title>WebServ</title>\n"
//         "<style>\n"
//         "body { font-family: Arial, sans-serif; background: #f4f4f4; display: flex; flex-direction: column; align-items: center; justify-content: center; height: 100vh; margin: 0; }\n"
//         "h1 { color: #333; margin-bottom: 20px; }\n"
//         "button { padding: 10px 20px; background: #333; color: white; border: none; border-radius: 5px; cursor: pointer; }\n"
//         "button:hover { background: #555; }\n"
//         "form input { padding: 8px; margin-bottom: 10px; border-radius: 5px; border: 1px solid #ccc; }\n"
//         "</style>\n"
//         "</head>\n"
//         "<body>\n"
//         "<h1>Hello, my name is Ali</h1>\n"
//         "<form method=\"POST\" action=\"/\">\n"
//         "<input type=\"text\" name=\"name\" placeholder=\"Enter your name\" />\n"
//         "<br/>\n"
//         "<button type=\"submit\">Submit</button>\n"
//         "</form>\n"
//         "</body>\n"
//         "</html>\n";
// }

// int main() {
//     int server_fd, client_fd;
//     struct sockaddr_in address;
//     socklen_t addrlen = sizeof(address);
//     char buffer[8192];

//     // Create socket
//     server_fd = socket(AF_INET, SOCK_STREAM, 0);
//     if (server_fd == -1) {
//         perror("socket");
//         return 1;
//     }

//     // Prepare address struct
//     memset(&address, 0, sizeof(address));
//     address.sin_family = AF_INET;
//     address.sin_addr.s_addr = INADDR_ANY;
//     address.sin_port = htons(PORT);

//     // Bind
//     if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
//         perror("bind");
//         close(server_fd);
//         return 1;
//     }

//     // Listen
//     if (listen(server_fd, 5) < 0) {
//         perror("listen");
//         close(server_fd);
//         return 1;
//     }

//     std::cout << "Server listening on http://0.0.0.0:" << PORT << std::endl;

//     while (1) {
//         client_fd = accept(server_fd, (struct sockaddr*)&address, &addrlen);
//         if (client_fd < 0) {
//             perror("accept");
//             continue;
//         }

//         memset(buffer, 0, sizeof(buffer));
//         int bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
//         if (bytes_received <= 0) {
//             close(client_fd);
//             continue;
//         }

//         std::string request(buffer);
//         std::string body = "";
//         std::string response;

//         if (request.substr(0, 4) == "POST") {
//             // Get Content-Length
//             size_t content_length_pos = request.find("Content-Length:");
//             if (content_length_pos != std::string::npos) {
//                 size_t value_start = request.find_first_of("0123456789", content_length_pos);
//                 size_t value_end = request.find("\r\n", value_start);
//                 int content_length = std::atoi(request.substr(value_start, value_end - value_start).c_str());

//                 size_t body_start = request.find("\r\n\r\n");
//                 if (body_start != std::string::npos) {
//                     body = request.substr(body_start + 4, content_length);
//                     std::cout << "[POST DATA] → " << body << std::endl;
//                 }
//             }

//             std::string success_page = 
//                 "<html><body><h1>POST Received</h1><p>Data: " + body + "</p></body></html>";
//             response =
//                 "HTTP/1.1 200 OK\r\n"
//                 "Content-Type: text/html\r\n"
//                 "Content-Length: " + std::to_string(success_page.size()) + "\r\n"
//                 "\r\n" + success_page;
//         } else {
//             std::string html = html_page();
//             response =
//                 "HTTP/1.1 200 OK\r\n"
//                 "Content-Type: text/html\r\n"
//                 "Content-Length: " + std::to_string(html.size()) + "\r\n"
//                 "\r\n" + html;
//         }

//         send(client_fd, response.c_str(), response.size(), 0);
//         close(client_fd);
//     }

//     close(server_fd);
//     return 0;
// }
