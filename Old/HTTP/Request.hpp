#include <unistd.h> 
#include <string>
#include <cstring>    
#include <iostream>
#include <sstream>
#include <map>
#include "../Webserver.hpp"

struct HttpRequest {
    std::string method;
    std::string path;
    std::string version;
    std::map<std::string, std::string> headers;
    std::string body;
};