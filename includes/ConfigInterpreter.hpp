/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigInterpreter.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: youness <youness@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 15:03:57 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/29 11:56:56 by youness          ###   ########.fr       */
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
#include <set>

class ConfigInterpreter {
    private:
        std::vector<ServerConfig> serverConfigs;
        std::vector<std::string> ConfigData;
        int serverFlag;
        int routeFlag;
        char **envp;

        void parseRouteBlock(RouteConfig& route, const std::string& line);
        void parseRouteOption(RouteConfig& route, const std::string& key, const std::string& value);
        void parseMethodsOption(RouteConfig& route, const std::string& value);
        void parseDirectoryListingOption(RouteConfig& route, const std::string& value);
        void parseUploadPathOption(RouteConfig& route, const std::string& value);
        void parseCgiOption(RouteConfig& route, const std::string& value);
        void parseServerBlock(ServerConfig& server, const std::string& line);
        void parseServerOption(ServerConfig& server, const std::string& key, const std::string& value, const std::string& line);
        void parseHostOption(ServerConfig& server, const std::string& value);
        void parsePortOption(ServerConfig& server, const std::string& value);
        void parseServerNameOption(ServerConfig& server, const std::string& value);
        void parseClientMaxBodySizeOption(ServerConfig& server, const std::string& value);
        void parseErrorPageOption(ServerConfig& server, const std::string& line);
        std::string extractPathFromRouteLine(const std::string& line);
        std::vector<std::string> readFile(const std::string& filepath);
        bool hasValidExtension(const std::string& filePath) const;
        std::string trim(const std::string& line);
        std::string clean_line(std::string line);
        bool Isspaces(const std::string& line);
        bool IsComment(const std::string& line);
        void printConfig() const;
        std::string toLower(std::string str);
    
    public:
        ConfigInterpreter();
        void getConfigData(std::string filePath);
        void parse();
        const std::vector<ServerConfig>& getServerConfigs() const;
        void checkValues();
        std::string getPathForCGI(char **envp) const;
    
};

#endif