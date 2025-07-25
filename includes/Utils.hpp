/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: baouragh <baouragh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/14 12:00:00 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/25 18:26:18 by baouragh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
#define UTILS_HPP

#include "../includes/HttpServer.hpp"

#include <string>
#include <sstream>

namespace Utils
{
    template<typename T>
    std::string toString(const T& value) {
        std::stringstream ss;
        ss << value;
        return ss.str();
    }
}
bool shouldKeepAlive(const HttpRequest& request);
void setConnectionHeaders(HttpResponse& response, bool keepAlive);
std::string toLower(const std::string& s);

void ltrim(std::string& s);
HttpResponse parseCgiOutput(const std::string& raw);
std::string buildSetCookieHeader(const std::string& session_id);

#endif