/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: Invalid date        by                   #+#    #+#             */
/*   Updated: 2025/05/05 11:36:46 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/* ************************************************************************** */

# include "Webserver.hpp"


int set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int main(int ac, char **av)
{
    if (ac != 2) {
        std::cerr << "Error:\n[usage]-> ./webserv configFile.config." << std::endl;
        return 1;
    }

    WebServ data(av[1]);

    try {
        data.parseConfig();
        data.printConfig();
        data.checkValues();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    std::vector<Server> servers;
    size_t size = data.getServerBlocks().size();
    Socket socks[size];

    for (size_t i = 0; i < size; ++i)
    {
        //*1 here we create the socket
        socks[i].setSocket(AF_INET, SOCK_STREAM, 0);
        Server server(socks[i]);

        // std::cout << "Socket created with fd: " << server.getSocket().getFd() << std::endl;

        sockaddr_in addr;
        std::memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(data.getServerBlocks()[i].port);
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
        servers.push_back(server);
    }

    std::cout << GREEN"Servers running..." << RES << std::endl;

    // *6 Create pollfd list
    std::vector<pollfd> poll_fds;
    pollfd pfd;
    for (size_t i = 0; i < servers.size(); ++i) {
        pfd.fd = servers[i].getSocket().getFd();
        pfd.events = POLLIN;
        pfd.revents = 0;
        poll_fds.push_back(pfd);
    }

    // Event loop
    while (true) {
        int ret = poll(poll_fds.data(), poll_fds.size(), -1);
        if (ret < 0) {
            perror("poll");
            break;
        }

        for (size_t i = 0; i < poll_fds.size(); ++i)
        {
            if (poll_fds[i].revents & POLLIN) {
                sockaddr_in client_addr;
                socklen_t addr_len = sizeof(client_addr);
                int client_fd = accept(poll_fds[i].fd, (struct sockaddr*)&client_addr, &addr_len);
                if (client_fd < 0) {
                    perror("accept");
                    continue;
                }
                std::string response =
                                "HTTP/1.1 200 OK\r\n"
                                "Content-Type: text/plain\r\n"
                                "Content-Length: 12\r\n"
                                "\r\n"
                                "Hello world\r\n";

                // std:: cout << "******************************\n";
                // std::cout << response << std::endl;
                // std:: cout << "******************************\n";

                send(client_fd, response.c_str(), response.size(), 0);
                // close(client_fd);
                // here i will receive the request
                char buffer[1024];
                ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
                if (bytes_received > 0) {
                    buffer[bytes_received] = '\0'; // Null-terminate the received data
                    std::cout << "Received request:\n" << buffer << std::endl;
                } else if (bytes_received < 0) {
                    perror("recv");
                }
                close(client_fd);
                std::cout << "Client disconnected" << std::endl;
            }
        }
    }

    return 0;
}
