#include "cgi.hpp"


// char **convert()
// {
//     ;
// }


char **set_env(Request &req)
{

    std::string query;

    query = req.path.find("?");
    std::cout << query << "\n";
    exit(1);
    std::map<std::string, std::string> env;
    if (req.method == GET)
    {
        env["REQUEST_METHOD"] = GET;
        env["QUERY_STRING"] = req.query; // for GET
        env["CONTENT_LENGTH"] = "0"; // GET and POST
    }
    else if (req.method == POST)
    {
        ;
    }
    env["SCRIPT_NAME"] = "/cgi-bin/test.py";
    env["GATEWAY_INTERFACE"] = "CGI/1.1";
    env["SERVER_PROTOCOL"] = "HTTP/1.1";
    env["SERVER_NAME"] = "localhost";
    env["SERVER_PORT"] = "8080";
    env["REMOTE_ADDR"] = "127.0.0.1";
    env["HTTP_USER_AGENT"] = "curl/7.68.0";
    return(NULL);
}

void cgi_runer(Request &req)
{
    // check req type 
    std::cout << req.method << "\n";
    char **env = set_env(req);
    (void)env;


    pid_t pid;
    std::string str = "." + req.path;
    char *argv[] = { (char *)str.c_str(), NULL };
    std::cout << (char *)str.c_str() << std::endl;

    pid = fork();
    if (!pid)
    {
        execve((char *)str.c_str(), argv, NULL);
        perror("execve failed");
        exit(1);
    }
    else if (pid > 0)
        wait(NULL);
    else
        std::cerr << "Fork failed" << std::endl;
        
}