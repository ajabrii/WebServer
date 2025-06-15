#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <unistd.h>
#include <string>
#include <cstring>
#include <iostream>
#include <sstream>
#include <map>
# include <sys/socket.h>
# define GET "GET"
# define POST "POST"
# define DELETE "DELETE"


#define BUFFER_SIZE 1024
#define N_EXTENTION 3


//[.py, .php]
class Request
{
    public:
        static std::string exts[N_EXTENTION];
        std::string method;
        std::string path;
        std::string version;
        std::string query;
        std::string script_path;
        std::map<std::string, std::string> headers;
        std::string body;
        char requesto[BUFFER_SIZE];
        bool isComplate;
        bool hasCgi;
        bool hasQurey;

    public:
        Request() {};
        ~Request() {};
        void parseHttpRequest();
        void sendResponse(int fd, const std::string &response);
        //here we will generate the response
        void generateResponse(int fd);
        void check_cgi();
};

#endif