#include "../includes/SessionManager.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

SessionManager::SessionManager(const std::string& directory)
: sessionDir(directory) {}

SessionManager::SessionManager()
: sessionDir("/tmp/sessions") {}

void SessionManager::save(const std::string& session_id, const std::map<std::string, std::string>& data) 
{
    std::ofstream file(("./session/" + session_id).c_str());
    if (!file) 
        return;
    for (std::map<std::string, std::string>::const_iterator it = data.begin(); it != data.end(); ++it) 
    {
        file << it->first << "=" << it->second << "\n";
    }
}

std::map<std::string, std::string> SessionManager::load(const std::string& session_id) 
{
    std::map<std::string, std::string> data;
    std::ifstream file(("./session/" + session_id).c_str());
    if (!file)
        return data;
    std::string line;
    while (std::getline(file, line)) 
    {
        std::string::size_type eq = line.find('=');
        if (eq != std::string::npos)
            data[line.substr(0, eq)] = line.substr(eq + 1);
    }
    return data;
}

std::string SessionManager::buildSessionFilePath(const std::string& session_id) const 
{
    return  "./session/" + session_id;
}