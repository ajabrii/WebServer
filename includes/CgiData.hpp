/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiData.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/02 09:56:26 by ajabri            #+#    #+#             */
/*   Updated: 2025/08/02 09:56:27 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>

class CgiData
{
    public:
        std::string query;
        std::string script_path;
        std::string cgi_extn;
        std::string CgiInterp;
        std::string PathInfo;
        bool hasCgi;

    public:
        CgiData();
        void DebugPrint(void);

};