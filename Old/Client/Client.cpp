/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/07 10:54:05 by ajabri            #+#    #+#             */
/*   Updated: 2025/05/07 11:03:42 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "Client.hpp"

Client::Client()
{
    // m_Client_fd = -1;
}

Client::Client(int fd, sockaddr_in addr, socklen_t addrLen)
{
    m_Client_fd = fd;
    m_Client_add = addr;
    m_addrLen = addrLen;
}

Client::~Client()
{
    // close(m_Client_fd);
}

int Client::getClientFD()
{
    return m_Client_fd;
}

void Client::setClientFD(int fd)
{
    m_Client_fd = fd;
}

// void Client::setClientAddr()

