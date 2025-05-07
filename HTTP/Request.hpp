#include <unistd.h>
#include <string>
#include <cstring>
#include <iostream>
#include <sstream>
#include <map>
// #include "../Webserver.hpp"

#define BUFFER_SIZE 1024

// struct HttpRequest
// {
//     std::string method;
//     std::string path;
//     std::string version;
//     // std::map<std::string, std::string> headers;
//     std::string body;

// };


class Request
{
    public:
        std::string method;
        std::string path;
        std::string version;
        std::map<std::string, std::string> headers;
        std::string body;
        char requesto[BUFFER_SIZE];
        bool isComplate;

    public:
        Request() {};
        ~Request() {};
        void parseHttpRequest();
};