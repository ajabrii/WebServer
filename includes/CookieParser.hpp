// CookieParser.hpp
#ifndef COOKIE_PARSER_HPP
#define COOKIE_PARSER_HPP

#include <string>
#include <map>

class CookieParser 
{
public:
    static std::map<std::string, std::string> parse(const std::string &header);
};

#endif
