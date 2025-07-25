#include "../includes/CookieParser.hpp"
#include <sstream>

std::map<std::string, std::string> CookieParser::parse(const std::string &header) 
{
    std::map<std::string, std::string> cookies;
    std::string::size_type start = 0, end;

    while ((end = header.find(';', start)) != std::string::npos) 
    {
        std::string token = header.substr(start, end - start);
        std::string::size_type eq = token.find('=');
        if (eq != std::string::npos)
            cookies[token.substr(0, eq)] = token.substr(eq + 1);
        start = end + 1;
        while (start < header.size() && header[start] == ' ')
            ++start;
    }

    // Handle last (or only) cookie
    std::string token = header.substr(start);
    std::string::size_type eq = token.find('=');
    if (eq != std::string::npos)
        cookies[token.substr(0, eq)] = token.substr(eq + 1);

    return cookies;
}
