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
        void sendResponse(int fd, const std::string &response);
        //here we will generate the response
        void generateResponse(int fd);
};