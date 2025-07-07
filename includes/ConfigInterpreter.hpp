/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigInterpreter.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: baouragh <baouragh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 15:03:57 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/07 15:14:40 by baouragh         ###   ########.fr       */
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
        std::string getPathForCGI(char **envp) const;
    
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
        std::string toLower(std::string str);
};

#endif
