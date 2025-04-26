/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/25 16:39:13 by ajabri            #+#    #+#             */
/*   Updated: 2025/04/26 11:15:14 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# ifndef WEBSERV_HPP
#define WEBSERV_HPP

# include <iostream>
# include <fstream>
# include <string>
#include <cstdlib>
# include <vector>
# include <cctype>
# include <map>

typedef enum tokens_e {
    TOKEN_SERVER_BLOCK,
    TOKEN_LOCATION_BLOCK,
    TOKEN_LISTEN,
    TOKEN_SERVER_NAME,
    TOKEN_ERROR_PAGE,
    TOKEN_CLIENT_MAX_BODY_SIZE,
    TOKEN_ROOT,
    TOKEN_INDEX,
    TOKEN_METHODS,
    TOKEN_AUTOINDEX,
    TOKEN_UPLOAD,
    TOKEN_CGI_PASS,

    TOKEN_LBRACE,      // {
    TOKEN_RBRACE,      // }
    TOKEN_SEMICOLON,   // ;
    TOKEN_UNKNOWN,     // fallback
    TOKEN_VALUE        // any literal string/number
} e_tokens;

typedef struct token_s {
    e_tokens type;
    std::string value;
    size_t line;
} t_token;


class WebServ
{
    private:
        std::string m_FileName;
        std::vector<std::string> m_ConfigData;

    public:
        WebServ();
        WebServ(std::string config);
        WebServ(const WebServ& other);
        WebServ& operator=(const WebServ& other);
        ~WebServ();
        std::string GetFileName();
        void OpenConfigFile(std::fstream& configFile);
        void ReadConfig(std::fstream& configFile);
        std::vector<std::string> GetConfigData();
        static void ServerLogs(std::string logs);
        static void initGrammerMap();
        static std::map<e_tokens, std::string> GrammerMap;
        
};

# endif