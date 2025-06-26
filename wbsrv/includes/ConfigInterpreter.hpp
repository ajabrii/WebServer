/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigInterpreter.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 15:03:57 by ajabri            #+#    #+#             */
/*   Updated: 2025/06/26 15:45:22 by ajabri           ###   ########.fr       */
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

class ConfigInterpreter {
    private:
        std::vector<ServerConfig> serverConfigs;
    
    public:
        ConfigInterpreter();
        void parse(const std::string& filepath);
        const std::vector<ServerConfig>& getServerConfigs() const;
    
    private:
        void parseServerBlock(const std::vector<std::string>& lines, size_t& i);
        void parseRouteBlock(ServerConfig& server, const std::vector<std::string>& lines, size_t& i);
        std::vector<std::string> readFile(const std::string& filepath);
        std::string trim(const std::string& line);
};

#endif
