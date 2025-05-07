/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/04 13:34:44 by ajabri            #+#    #+#             */
/*   Updated: 2025/05/07 11:02:39 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

// # include "../Webserver.hpp"
#include <sys/socket.h> 
#include <netinet/in.h> 

class Client
{
    private:
        int m_Client_fd; // client file descriptor
        sockaddr_in m_Client_add;
        socklen_t m_addrLen;
    public: 
        Client();
        Client(int fd, sockaddr_in addr, socklen_t addrLen);
        ~Client();
        int getClientFD();
        void setClientFD(int fd);
        // void setClientAddr()
};