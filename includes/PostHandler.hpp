/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PostHandler.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/01 18:16:22 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/30 11:25:53 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

# include "IHttpMethodHandler.hpp"
# define POST_LOG "\033[1;36m[POST]\033[0m"
struct Part {
    std::string name;
    std::string filename;
    std::string content;
};

class PostHandler : public IHttpMethodHandler
{

    private:
        std::string extractBoundary(const std::string &ct) const;
        void writeFile(const std::string &path, const std::string &content) const;
        void writeKeyValuesToFile(const std::string &path, const std::map<std::string, std::string> &fields) const;
        std::vector<Part> parseMultipart(const std::string &body, const std::string &boundary) const;
        std::map<std::string, std::string> parseFormUrlEncoded(const std::string &body) const;
        HttpResponse makeErrorResponse(int statusCode, const std::string &message, const ServerConfig& serverConfig) const;
        std::string DataDecode(const std::string &encoded) const;

    public:
        PostHandler();
        ~PostHandler();
        virtual HttpResponse handle(const HttpRequest &req, const RouteConfig& route, const ServerConfig& serverConfig) const;
};
