/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigInterpreter.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ytarhoua <ytarhoua@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 15:03:57 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/05 20:17:31 by ytarhoua         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_INTERPRETER_HPP
#define CONFIG_INTERPRETER_HPP

// #include <vector>
// #include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include "ServerConfig.hpp"
#include <cstdlib>
#include <string>
#include <vector>
#include <map>

class ConfigInterpreter {
    private:
        std::vector<ServerConfig> serverConfigs;
        std::vector<std::string> ConfigData;
        int serverFlag;
        int routeFlag;
        char **envp;
    
    public:
        ConfigInterpreter();
        void getConfigData(std::string filePath);
        void parse();
        const std::vector<ServerConfig>& getServerConfigs() const;
        void checkValues() const;
        std::string getPathForCGI(char **envp)
        {
            std::string str;
            
            if (!envp || !*envp)
                return "";
            for (size_t i = 0; envp[i] != NULL; i++)
            {
                str = envp[i];
                if (str.find("PATH=") != std::string::npos && !(str.find("_PATH") != std::string::npos))
                {
                    return envp[i];
                }
            }
            return "";
        };
    
    private:
        // void parseServerBlock(const std::vector<std::string>& lines, size_t& i);
        // void parseRouteBlock(ServerConfig& server, const std::vector<std::string>& lines, size_t& i);
        void parseRouteLine(RouteConfig& route, const std::string& line);
        void parseServerLine(ServerConfig& server, const std::string& line);
        std::string extractPathFromRouteLine(const std::string& line);
        std::vector<std::string> readFile(const std::string& filepath);
        std::string trim(const std::string& line);
        std::string clean_line(std::string line);
        bool Isspaces(const std::string& line);
        bool IsComment(const std::string& line);
        void printConfig() const;
};

#endif
