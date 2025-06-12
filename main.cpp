/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: baouragh <baouragh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: Invalid date        by                   #+#    #+#             */
/*   Updated: 2025/06/12 15:30:36 by baouragh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/* ************************************************************************** */

# include "Webserver.hpp"

int main(int ac, char **av)
{
    if (ac != 2) {
        WebServ::logs("Error:\n[usage]-> ./webserv configFile.config.");
        return 1;
    }
    WebServ Websev(av[1]);
    try {
        Websev.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}


    // std::vector<Server> servers;
    // size_t size = data.getServerBlocks().size();
    // Socket socks[size];

    // for (size_t i = 0; i < size; ++i)
    // {
    //     //*1 here we create the socket
    //     socks[i].setSocket(AF_INET, SOCK_STREAM, 0);
    //     Server server(socks[i]);

    //     // std::cout << "Socket created with fd: " << server.getSocket().getFd() << std::endl;

    //     sockaddr_in addr;
    //     std::memset(&addr, 0, sizeof(addr));
    //     addr.sin_family = AF_INET;
    //     addr.sin_port = htons(data.getServerBlocks()[i].port);
    //     // std::cout << data.getServerBlocks()[i].port << std::endl;
    //     addr.sin_addr.s_addr = INADDR_ANY;

    //     //*2 here we create the address and bind it to the socket
    //     if (bind(socks[i].getFd(), (struct sockaddr*)&addr, sizeof(addr)) < 0) {
    //         perror("bind");
    //         close(socks[i].getFd());
    //         continue;
    //     }
    //     //*3 here we listen to the socket
    //     if (listen(socks[i].getFd(), CLIENT_QUEUE) < 0) {
    //         perror("listen");
    //         close(socks[i].getFd());
    //         continue;
    //     }
    //     //*4 here we set the socket to non-blocking
    //     if (set_nonblocking(socks[i].getFd()) < 0) {
    //         perror("fcntl");
    //         close(socks[i].getFd());
    //         continue;
    //     }

    //     // std::cout << "Server listening on port " << data.getServerBlocks()[i].port << std::endl;
    //     server.setServerAddress(addr); // implement this if not already
    //     //*5 storing the server block to the server vector
    //     servers.push_back(server);
    // }

    // std::cout << GREEN"Servers running..." << RES << std::endl;

    // *6 Create pollfd list
    // std::vector<pollfd> poll_fds;
    // std::map<int, bool> isServFD;
    // pollfd pfd;
    // std::vector<Server> servers = data.getServers();
    // for (size_t i = 0; i < servers.size(); ++i) {
    //     pfd.fd = servers[i].getSocket().getFd();
    //     isServFD[pfd.fd] = true;
    //     pfd.events = POLLIN;
    //     pfd.revents = 0;
    //     poll_fds.push_back(pfd);
    // }

    // !Event loop
    // std::map<int, Client> clients;
    // while (true) {
    //     int ret = poll(poll_fds.data(), poll_fds.size(), -1);
    //     if (ret < 0) {
    //         perror("poll");
    //         break;
    //     }

    //     for (size_t i = 0; i < poll_fds.size(); ++i)
    //     {
    //         if (poll_fds[i].revents & POLLIN) {

    //             if (isServFD[poll_fds[i].fd] == true)
    //             {

    //                 sockaddr_in client_addr;
    //                 socklen_t addr_len = sizeof(client_addr);
    //                 int client_fd = accept(poll_fds[i].fd, (struct sockaddr*)&client_addr, &addr_len);
    //                 if (client_fd < 0) {
    //                     std::cerr << "Error accepting connection  " << client_fd<<std::endl;
    //                     // perror("accept");
    //                     continue;
    //                 } else {
    //                     if (set_nonblocking(client_fd) < 0) {
    //                         perror("fcntl");
    //                         close(client_fd);
    //                         continue;
    //                     }
    //                     clients[client_fd] = Client(client_fd, client_addr, addr_len);
    //                     pollfd clintpfd;
    //                     clintpfd.fd = client_fd;
    //                     clintpfd.events = POLLIN;
    //                     clintpfd.revents = 0;
    //                     poll_fds.push_back(clintpfd);
    //                     std::cout << "New client connected: " << inet_ntoa(client_addr.sin_addr) << ":" << ntohs(client_addr.sin_port) << std::endl;
    //                 }
    //             }
    //         // *1. receive the request
    //             } else if (isServFD[poll_fds[i].fd] == false) {
    //                 // here i will receive the request
    //                 int client_fd = poll_fds[i].fd;
    //                 char buffer[1024];
    //                 ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    //                 if (bytes_received > 0) {
    //                     buffer[bytes_received] = '\0'; // Null-terminate the received data
    //                     std::cout << "Received request:\n" << buffer << std::endl;

    //                     // *2. send the response
    //                     std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!";
    //                     send(client_fd, response.c_str(), response.size(), 0);

    //                 } else if (bytes_received < 0) {
    //                         // !~remove the client from the poll_fds and clients map (disconnect ones)
    //                         std::cerr << "Error receiving data from client: " << client_fd << std::endl;
    //                         close(client_fd);
    //                         poll_fds.erase(poll_fds.begin() + i);
    //                         clients.erase(client_fd);
    //                         isServFD.erase(client_fd);
    //                         i--;
    //                     perror("recv");
    //                 }
    //             }
    //         }
    //     }