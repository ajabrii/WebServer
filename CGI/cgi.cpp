#include "cgi.hpp"


void printEnvp(char** envp) 
{
    for (char** p = envp; *p != NULL; ++p) {
        std::cout << *p << std::endl;
    }
}

char **convert_env(std::map<std::string, std::string> env)
{
    size_t size = env.size();
    char **envp = new char*[size + 1];

    size_t i = 0;
    for (std::map<std::string, std::string>::const_iterator it = env.begin(); it != env.end(); ++it) 
    {
        std::string entry = it->first + "=" + it->second;
        envp[i] = new char[entry.size() + 1];
        std::copy(entry.begin(), entry.end(), envp[i]);
        envp[i][entry.size()] = '\0';

        ++i;
    }
    envp[i] = NULL;
    printEnvp(envp);
    return envp;
}



char **set_env(Request &req)
{

    std::string query;

    query = req.path.find("?");
    std::cout << query << "\n";
    std::map<std::string, std::string> env;
    if (req.method == GET)
    {
        env["REQUEST_METHOD"] = GET;
        env["QUERY_STRING"] = req.query; // for GET
        env["CONTENT_LENGTH"] = "0"; // GET and POST
    }
    else if (req.method == POST)
    {
        // env["CONTENT_LENGTH"] = req.length;
        // env["CONTENT_TYPE"] = "application/x-www-form-urlencoded";
    }
    env["SCRIPT_NAME"] = req.script_path;
    env["GATEWAY_INTERFACE"] = "CGI/1.1";
    env["SERVER_PROTOCOL"] = "HTTP/1.1";
    env["SERVER_NAME"] = "localhost";
    // env["SERVER_PORT"] = "8080"; // port of server
    // env["REMOTE_ADDR"] = "127.0.0.1"; // addrss of remote
    env["HTTP_USER_AGENT"] = "curl/7.68.0";
    return (convert_env(env));
}



void cgi_runer(Request &req) // /cgi-bin/file.php
{
    // check req type 
    std::cout << req.method << "\n";
    char **env = set_env(req);
    // (void)env;


    pid_t pid;
    std::string str = req.script_path.substr(1);
    std::string cmd("python3");
    char *argv[] = { (char *)cmd.c_str(), (char *)str.c_str() ,NULL };
    std::cout << "7777777777777777777777777->>> " << (char *)cmd.c_str() << "    " <<(char *)str.c_str() << std::endl;

    pid = fork();
    if (!pid)
    {
        execve("/usr/bin/python3", argv, env);
        perror("execve failed");
        exit(1);
    }
    // listen to the output and store it to pipe
    else if (pid > 0)
        wait(NULL);
    else
        std::cerr << "Fork failed" << std::endl;
        
}