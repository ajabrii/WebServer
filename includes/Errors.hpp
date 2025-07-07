/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Errors.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/07 11:30:56 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/07 11:34:30 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# pragma once

#pragma once

#include <string>
#include "ServerConfig.hpp"

class Error {
public:
    static std::string loadErrorPage(int statusCode, const ServerConfig& config);
    static void logs(const std::string& message);
};

