/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/04 13:34:44 by ajabri            #+#    #+#             */
/*   Updated: 2025/05/04 15:04:41 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

// # include "../Webserver.hpp"

class Client
{
    private:
        int m_Client_fd; // client file descriptor
        // sockaddr_in m_Client_add;
        // socklen_t m_addrLen;
    public: 
        Client();
        ~Client();
        int getClientFD();
        void setClientFD(int fd);
        // void setClientAddr()
};