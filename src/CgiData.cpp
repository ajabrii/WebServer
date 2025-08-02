/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiData.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/02 10:00:49 by ajabri            #+#    #+#             */
/*   Updated: 2025/08/02 10:00:50 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/CgiData.hpp"

CgiData::CgiData() : query(""), script_path(""), cgi_extn(""), PathInfo("") ,hasCgi(false)
{

}

void CgiData::DebugPrint(void)
{

    std::cerr << "CgiExt: " << this->cgi_extn  << std::endl
    << "hasCgi: " << this->hasCgi  << std::endl
    << "PathInfo: " << this->PathInfo  << std::endl
    << "query: " << this->query  << std::endl
    << "script_path: " << this->script_path << std::endl
    << "Cgi interp: " << this->CgiInterp << std::endl;
}
