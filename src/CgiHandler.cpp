/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/01 19:44:18 by baouragh          #+#    #+#             */
/*   Updated: 2025/07/14 14:38:08 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "../includes/CgiHandler.hpp"
# include "../includes/CgiData.hpp"
#include "../includes/Utils.hpp"
#include <sstream>
#include <algorithm>
#include <cctype> 


CgiHandler::CgiHandler() {}

CgiHandler::~CgiHandler() {}

CgiHandler::CgiHandler(const HttpServer &server, const HttpRequest& req , const RouteConfig& route ,int clientSocket, std::string env_paths)
{
    _serverData = server.getConfig();
    _route = route;
    _req = req;
    _env_paths = env_paths;
    // _serverSocket = server.getFd(); // it should be a vector later on
    _clientSocket = clientSocket;
    _data = check_cgi();
    // _data.DebugPrint();
}
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
        // std::copy(entry.begin(), entry.end(), envp[i]);
        std::strcpy(envp[i], entry.c_str());
        envp[i][entry.size()] = '\0';

        ++i;
    }
    envp[i] = NULL;
    // printEnvp(envp);
    return envp;
}



// char **set_env(const HttpRequest &req,CgiData& data)
// {

//     std::string query;

//     // query = req.path.find("?");
//     std::cout << query << "\n";
//     std::map<std::string, std::string> env;
//     if (req.method == GET)
//     {
//         env["REQUEST_METHOD"] = GET;
//         env["QUERY_STRING"] = data.query; // for GET
//         env["CONTENT_LENGTH"] = "0"; // GET and POST
//     }
//     else if (req.method == POST)
//     {
//         env["REQUEST_METHOD"] = POST;
//         env["CONTENT_LENGTH"] = req.body.length();
//         // env["CONTENT_TYPE"] = "application/x-www-form-urlencoded";
//     }
//     env["SCRIPT_NAME"] = data.script_path;
//     env["SCRIPT_FILENAME"] = "." + data.script_path;
//     env["GATEWAY_INTERFACE"] = "CGI/1.1";
//     env["SERVER_PROTOCOL"] = "HTTP/1.1";
//     env["SERVER_NAME"] = "localhost";
//     env["REDIRECT_STATUS"] = "200";
//     // env["SERVER_PORT"] = "8080"; // port of server
//     // env["REMOTE_ADDR"] = "127.0.0.1"; // addrss of remote
//     env["HTTP_USER_AGENT"] = "curl/7.68.0";
//     return (convert_env(env));
// }

// In CgiHandler.cpp

char **CgiHandler::set_env(void)
{
    std::map<std::string, std::string> env_map; // Use env_map for clarity

    env_map["REQUEST_METHOD"] = _req.method;
    env_map["SCRIPT_NAME"] = _data.script_path;
    env_map["SCRIPT_FILENAME"] = "." + _data.script_path; // Assuming server root is current dir
    env_map["QUERY_STRING"] = _data.query;
    env_map["SERVER_PORT"] = Utils::toString(_serverData.port[0]); // ! port[0]
    env_map["REMOTE_ADDR"] = Utils::toString(_clientSocket); // Get client IP address
    
    env_map["GATEWAY_INTERFACE"] = "CGI/1.1";
    env_map["SERVER_PROTOCOL"] = "HTTP/1.1";
    env_map["SERVER_NAME"] = ""; // Get from server config
    std::cout << "Holaaaaaaaaaaaaa " << _req.GetHeader("host") << std::endl;
    env_map["REDIRECT_STATUS"] = "200"; // Common for web servers using CGI
    
    if (_req.method == GET) 
    {
        env_map["CONTENT_LENGTH"] = "0";
    }
    else if (_req.method == POST) 
    {
        std::ostringstream oss_cl;
        oss_cl << _req.body.length();
        env_map["CONTENT_LENGTH"] = oss_cl.str();
        env_map["CONTENT_TYPE"] = _req.GetHeader("Content-Type");;
    }
    else if (_req.method == DELETE)
    {
        ;
    }

    for (std::map<std::string, std::string>::const_iterator it = _req.headers.begin(); it != _req.headers.end(); ++it)
    {
        std::cout << "Key: " << it->first << ", Value: " << it->second << std::endl;
    }
    

    // // --- Convert ALL client HTTP headers to HTTP_UPPERCASE_VAR ---
    // for (std::map<std::string, std::string>::const_iterator it = _req.headers.begin(); it != _req.headers.end(); ++it) 
    // {
    //     std::string header_name = it->first;
    //     std::transform(header_name.begin(), header_name.end(), header_name.begin(), ::toupper); // Convert to uppercase
    //     std::replace(header_name.begin(), header_name.end(), '-', '_'); // Replace hyphens with underscores
    //     env_map["HTTP_" + header_name] = it->second;
    // }

    return (convert_env(env_map));
}

std::vector<std::string> split_string(const std::string& path, char c) // string=/path:/path:....
{
    std::vector<std::string> subs;
    size_t prev_pos = 0;
    size_t current_pos = 0;

    current_pos = path.find(c, prev_pos);
    while (current_pos != std::string::npos)
    {
        subs.push_back(path.substr(prev_pos, current_pos - prev_pos));
        prev_pos = current_pos + 1;
        current_pos = path.find(c, prev_pos);
    }
    subs.push_back(path.substr(prev_pos));

    // for (size_t i = 0; i < subs.size(); i++)
    // {
    //     std::cout << subs[i] << std::endl;
    // }
    return subs;
}

std::string full_path(std::string paths, std::string cmd)
{
    std::vector<std::string> subs =  split_string(paths, ':'); // bash
    subs[0] = subs[0].substr(subs[0].find('=') + 1);
    for (size_t i = 0; i < subs.size(); i++)
    {
        std::cout << "tetsing for -> \'" << (subs[i] + "/" + cmd) <<"\'" << std::endl;
        if (!access((subs[i] + "/" + cmd).c_str(), X_OK))
            return (subs[i] + "/" + cmd);
    }

    return ("");
}

CgiData CgiHandler::check_cgi(void)
{
    CgiData data;
    size_t pos;
    std::string founded_extns;
    bool is_query = false;
    std::string exts[3] = {".py", ".php", ".sh"}; // Config

    for (size_t i = 0; i < 3; i++)
    {
        
        pos = _req.uri.find(exts[i]);
        if (pos == std::string::npos)
            continue;
        founded_extns = _req.uri.substr(pos);
        std::cout << "-------------------------founded_extns---------------> `" << founded_extns << "'\n";  
        std::cout << "------------------------ ORGGG ---------------> `" << _req.uri << "'\n";
        // std::cout << "------------------founded_extns.substr(0, exts[i].size())----------------------> `" << founded_extns.substr(0, exts[i].size()) << "'\n";
        // std::cout << "-----------------------founded_extns.c_str()[exts[i].size()]-----------------> `" << founded_extns.c_str()[exts[i].size()] << "'\n";

        if (founded_extns == exts[i] || (is_query = (founded_extns.substr(0, exts[i].size()) == exts[i] && founded_extns.c_str()[exts[i].size()] == '?')))
        {
            std::cout << "-------------------------is_query---------------> `" << is_query << "'\n";  
            if (is_query)
            {
                std::cout << "||||||||||||||| --------> "<< _req.uri << std::endl;
                data.query = founded_extns.substr(exts[i].size() + 1); //
                data.script_path = _req.uri.substr(0, _req.uri.find('?'));
            }
            else
               data.script_path = _req.uri;
            data.hasCgi = true;
            data.cgi_extn = founded_extns.substr(0, exts[i].size());
        }
    }
    return (data);
}

bool CgiHandler::IsCgi(void)
{
    return (_data.hasCgi);
}

HttpResponse CgiHandler::execCgi(void)
{
    HttpResponse f;//
    std::string cmd;
    std::string fp;
    std::string str;
    pid_t pid;
    char **env;

    // data = check_cgi();
    // if (!data.hasCgi)
    //     ;
        // NO Cgi;
    env = set_env();
    
    // cmd = route.cgiExtension;
    // std::cout << "----------------CMD----------------->: " << cmd << std::endl;
    if (_data.cgi_extn == ".py")
        cmd = "python3"; // inter // Config
    else if (_data.cgi_extn == ".php")
        cmd = "php-cgi";
    else if (_data.cgi_extn == ".sh")
        cmd = "bash";

    fp  = full_path(_env_paths, cmd); // TO DO HANDLE ERROR

    std::cout << "FULL PATH" << fp << std::endl;
    // if (fp.empty())
    //     return;
    str = _data.script_path.substr(1);
    char *argv[] = { (char *)cmd.c_str(), (char *)str.c_str() ,NULL };
    // std::cout << "7777777777777777777777777->>> " << (char *)cmd.c_str() << "    " <<(char *)str.c_str() << std::endl;

    int pfd[2];
    if (pipe(pfd))
        exit(1);
    pid = fork();
    if (!pid)
    {
        dup2(pfd[1], 1);
        close(pfd[1]);
        close(pfd[0]);
    
        execve(fp.c_str(), argv, env);
        perror("execve failed");
        _exit(1);
    }
    // listen to the output and store it to pipe
    else if (pid < 0)
    {
        std::cerr << "Fork failed" << std::endl;
        close(pfd[0]);
        close(pfd[1]);
        // return;
    }
    close(pfd[1]);
    
    std::string cgi_output_str;
    char buffer[4096];
    ssize_t bytes_read;
    while ((bytes_read = read(pfd[0], buffer, 4096 - 1)) > 0) 
    {
        buffer[bytes_read] = '\0'; // Null-terminate the chunk
        cgi_output_str.append(buffer); // Append to the output string
    }
    if (bytes_read == -1)
    {
        perror("read from pipe_stdout failed");
    }
    close(pfd[0]);
    std::cout << "--------------------> " << cgi_output_str << std::endl;
    f.version = "HTTP/1.1";
    // f.headers["Content-Type"] = "text/html";
    f.statusCode = 200;
    f.body = cgi_output_str;
    wait(NULL);
    return f;   
}