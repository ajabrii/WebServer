# include "Webserver.hpp"
// include poll() header
# include <poll.h>

int main(int ac, char **av)
{
    WebServ data(av[1]);
    (void)ac;
    data.parseConfig();
    data.printConfig();
}

// int main(int ac, char **av)
// {

//     WebServ data(av[1]);
    
//     (void)av;
//     if (ac != 2) {
//        WebServ::ServerLogs("Error:\n[usage]-> ./webserv configFile.config.");
//         return (1);
//     }
//     std::fstream configFile;
//     data.ReadConfig(configFile);
//     std::cout << "=========================SERVER-BLOCK============================" << std::endl;

//     int sockFd;
//     int clientFd;
//     struct sockaddr_in address;
//     int addrslen = sizeof(address);

//     (void)clientFd; 
//     (void)addrslen;
//     // *1. Create socket (IPv4, TCP)
//     sockFd = socket(AF_INET, SOCK_STREAM, 0);
//     if (sockFd == -1) {
//         WebServ::ServerLogs("Error: socket creation failed");
//         return 1;
//     }
//     //*2. Set address info
//     memset(&address, 0, sizeof(address)); // we memset the address to 0 so we can avoid garbage values
//     address.sin_family = AF_INET; // sin_family = AF_INET means we are using IPv4
//     address.sin_addr.s_addr = INADDR_ANY; // sin_addr = INADDR_ANY means we are using all interfaces default interface
//     address.sin_port = htons(8081); // htons() stands for host to network short, it converts the port number from host byte order to network byte order
//     //*3. Bind socket to address

//     //? bind() associates the socket with the address and port number specified in the sockaddr_in structure (example: address = 192.168.43.1 and port = 8080sd )
//     if (bind(sockFd,(struct sockaddr *)&address, sizeof(address)) == -1) {
//         WebServ::ServerLogs("Error: bind failed");
//         close(sockFd);
//         return 1;
//     }
    
//     //*4. Listen
//     while (1) {
        
//         if (listen(sockFd, 5) == -1) {
//             WebServ::ServerLogs("Error: listen failed");
//             close(sockFd);
//             return 1;
//         }
//         // WebServ::ServerLogs(GREEN"Server listening on pot 8080..."RES);
//         // ? listen accepts in it parameter the number of clients that can be queued to connect to the server. and the socket fd.
//         //*5. Accept loop (accept one client for now)
//         clientFd = accept(sockFd, (struct sockaddr *)&address, (socklen_t*)&addrslen);
//         if (clientFd == -1) {
//             WebServ::ServerLogs("Error: accept failed");
//             close(sockFd);
//             return 1;
//         }
//         // std::cout <<YELLOW<< "Client connected from " <<RES<<GREEN<< inet_ntoa(address.sin_addr) <<RES <<std::endl;
//         // ? accept() accepts a connection on the socket and creates a new socket for the client. It returns the new socket file descriptor.
//      // Here is your new HTML body
//         const char *html_body = 
//         "<!DOCTYPE html>\n"
//         "<html lang=\"en\">\n"
//         "<head>\n"
//         "<meta charset=\"UTF-8\">\n"
//         "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
//         "<title>ARPANET++</title>\n"
//         "<link rel=\"icon\" href=\"./www/t.png\" type=\"image/x-icon\">"
//         "</head>\n"
//         "<body>\n"
//         "<div style=\"display: flex; justify-content: center; align-items: center; height: 100vh; background-color: #f0f0f0;\">"
//             "<h1 style=\"display: flex; font-weight:200; flex-direction: column;\">Webserv repo </h1>\n"
//             "<button style=\"background-color: green; color: white; padding: 10px 20px; border: none; border-radius: 5px; margin-left: 10px\"><a href=\"https://github.com/ajabrii/WebServer/\" style=\"text-decoration: none; color: white;\">Click Me</a></button>"
//         "</div>"
//         "</body>\n"
//         "</html>\n";

//         // Create the full HTTP response
//         char response[4096];
//         sprintf(response,
//         "HTTP/1.1 200 OK\r\n"
//         "Content-Type: text/html\r\n"
//         "Content-Length: %lu\r\n"
//         "\r\n"
//         "%s",
//         strlen(html_body),
//         html_body);

//         // Send the HTTP response
//         send(clientFd, response, strlen(response), 0);

        
//         // ? send() sends the response to the client. It takes the socket file descriptor, the response string, the length of the response, and flags (0 in this case).
//         //*7. Close sockets
    
//         char buffer[1024];
//         int bytesReceived = recv(clientFd, buffer, sizeof(buffer) - 1, 0);
//         if (bytesReceived > 0) {
//             buffer[bytesReceived] = '\0'; // Null-terminate the received data
//             std::cout  << buffer << std::endl;
//         } else {
//             std::cerr << "Error receiving data" << std::endl;
//         }
//     }
//     // while (1)
//     //     ;
//     // ? poll() is used to wait for events on file descriptors. In this case, it waits for 1 second (1000 milliseconds) before continuing.
    
    
    
    
//     // ? recv() receives data from the client. It takes the socket file descriptor, a buffer to store the received data, the size of the buffer, and flags (0 in this case).
    
//     close(clientFd);
//     close(sockFd);
// }



// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>
// #include <sys/socket.h>
// #include <netinet/in.h>

// #define BUFFER_SIZE 1024

// int main() {
//     int server_fd, client_fd;
//     struct sockaddr_in server_addr, client_addr;
//     socklen_t client_len = sizeof(client_addr);
//     char buffer[BUFFER_SIZE];

//     //create socket
//     server_fd = socket(AF_INET, SOCK_STREAM, 0);
//     if (server_fd == -1) {
//         perror("Socket creation failed");
//         exit(EXIT_FAILURE);
//     }

//     // Set up server address
//     server_addr.sin_family = AF_INET;
//     server_addr.sin_addr.s_addr = INADDR_ANY;
//     server_addr.sin_port = htons(8080);

//     // Bind the socket
//     if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
//         perror("Bind failed");
//         close(server_fd);
//         exit(EXIT_FAILURE);
//     }

//     // now Listen
//     if (listen(server_fd, 5) < 0) {
//         perror("Listen failed");
//         close(server_fd);
//         exit(EXIT_FAILURE);
//     }

//     printf("Server is listening on port 8080...\n");

//     // Accept loop: handle one client at a time
//     while (1) {
//         client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
//         if (client_fd < 0) {
//             perror("Accept failed");
//             continue;
//         }

//         printf("Client connected!\n");

//         // clearing before use to avoid problems
//         memset(buffer, 0, BUFFER_SIZE);

//         // reading data that recived 
//         ssize_t bytes_read = read(client_fd, buffer, BUFFER_SIZE - 1);
//         if (bytes_read < 0) {
//             perror("Read failed");
//         } else {
//             buffer[bytes_read] = '\0';
//             printf("Received from client: %s\n", buffer);

//             // Send response
//             char *response = "Message received!\n";
//             send(client_fd, response, strlen(response), 0);
//         }

//         close(client_fd); // close client connection
//         printf("Client disconnected.\n");
//     }

//     // Close the server
//     close(server_fd);
//     return 0;
// }