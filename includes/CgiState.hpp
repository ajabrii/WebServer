/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiState.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/02 09:56:32 by ajabri            #+#    #+#             */
/*   Updated: 2025/08/02 10:00:09 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <sys/wait.h>
#include <iostream>


typedef class Connection Connection;
typedef class Reactor Reactor;
class CgiState
{
    public:
        CgiState();
        ~CgiState();
        pid_t pid;
        int output_fd;
        int input_fd;
        std::string rawOutput;
        time_t startTime;
        bool headersParsed;
        bool bodySent;
        size_t bytesWritten;
        std::string script_path;
        Connection* connection;
        void writeToScript(Connection& conn);
        void readFromScript(Connection& conn , Reactor& reactor);
};
