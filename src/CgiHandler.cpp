/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: baouragh <baouragh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/01 19:44:18 by baouragh          #+#    #+#             */
/*   Updated: 2025/07/30 17:32:36 by baouragh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "../includes/CgiHandler.hpp"
# include "../includes/CgiData.hpp"
#include "../includes/Utils.hpp"
#include <sstream>
#include <algorithm>
#include <cctype> 
#include <signal.h>
#include <unistd.h> 
#include <sys/epoll.h>
#include <ctime>


CgiHandler::CgiHandler() {}

CgiHandler::~CgiHandler() {}

CgiHandler::CgiHandler(const HttpServer &server, const HttpRequest& req , const RouteConfig& route ,int clientSocket, std::string env_paths)
{
    _serverData = server.getConfig();
    _route = route;
    _req = req;
    _env_paths = env_paths;
    _clientSocket = clientSocket;
    _data = check_cgi();
}
void printEnvp(char** envp) 
{
    for (char** p = envp; *p != NULL; ++p) {
        std::cout << *p << std::endl;
    }
}

void deleteEnvp(char** envp) 
{
    if (envp == NULL)
        return;
    for (size_t i = 0; envp[i] != NULL; ++i) 
    {
        delete[] envp[i];
    }
    delete[] envp;
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
        std::strcpy(envp[i], entry.c_str());
        envp[i][entry.size()] = '\0';

        ++i;
    }
    envp[i] = NULL;
    return envp;
}


char **CgiHandler::set_env(Connection &conn)
{
    std::map<std::string, std::string> env_map;
    std::string host = _req.GetHeader("host");

    for (std::map<std::string, std::string>::const_iterator it = _req.headers.begin();
         it != _req.headers.end(); ++it) 
    {
        std::string key = it->first;
        std::transform(key.begin(), key.end(), key.begin(), ::toupper);
        std::replace(key.begin(), key.end(), '-', '_');

        if (key != "CONTENT_TYPE" && key != "CONTENT_LENGTH")
            key = "HTTP_" + key;

        env_map[key] = it->second;
    }

    env_map["REQUEST_METHOD"] = _req.method;
    env_map["PATH_INFO"] = _data.PathInfo;
    env_map["SCRIPT_NAME"] = _data.script_path;
    env_map["SCRIPT_FILENAME"] = _data.script_path;
    env_map["QUERY_STRING"] = _data.query;
    env_map["REMOTE_ADDR"] = conn.getClientIP();
    env_map["GATEWAY_INTERFACE"] = "CGI/1.1";
    env_map["SERVER_PROTOCOL"] = "HTTP/1.1";
    env_map["REDIRECT_STATUS"] = "200";

    if (host.find(':') != std::string::npos)
    {
        env_map["SERVER_NAME"] = host.substr(0, host.find(":"));
        env_map["SERVER_PORT"] = host.substr(host.find(":") + 1);
    }
    else
    {
        env_map["SERVER_NAME"] = host;
        env_map["SERVER_PORT"] = "80"; // fallback
    }

    if (_req.method == "GET") 
    {
        env_map["CONTENT_LENGTH"] = "0";
    }
    else if (_req.method == "POST") 
    {
        std::ostringstream oss;
        oss << _req.body.size();
        env_map["CONTENT_LENGTH"] = oss.str();
        env_map["CONTENT_TYPE"] = _req.GetHeader("Content-Type");
    }

    return convert_env(env_map);
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
    return subs;
}

std::string full_path(std::string paths, std::string cmd) // /usr/bin/python3
{
    if (!access(cmd.c_str(), X_OK))
            return (cmd);
    std::vector<std::string> subs =  split_string(paths, ':'); // bash
    
    if (subs[0].find('=') != std::string::npos)
        subs[0] = subs[0].substr(subs[0].find('=') + 1);
    
    for (size_t i = 0; i < subs.size(); i++)
    {
        std::cout << "tetsing for -> \'" << (subs[i] + "/" + cmd) <<"\'" << std::endl;
        if (!access((subs[i] + "/" + cmd).c_str(), X_OK))
            return (subs[i] + "/" + cmd);
    }
    std::cerr << "Error: CGI interpreter not found : " << cmd << std::endl;
    
    return ("");
}

CgiData CgiHandler::check_cgi(void)
{
    CgiData data;
    data.hasCgi = false;
    size_t query_pos = _req.uri.find('?');
    bool has_query = false;

    if (query_pos != std::string::npos)
        has_query = true;

    if (_req.method != "GET" && _req.method != "POST")
        throw HttpRequest::HttpException(405, "Method Not Allowed");

    // Loop over CGI extension map
    for (std::map<std::string, std::string>::const_iterator it = _route.cgi.begin(); it != _route.cgi.end(); ++it)
    {
        const std::string& ext = it->first;
        const std::string& interp = it->second;

        size_t ext_pos = _req.uri.find(ext);
        if (ext_pos == std::string::npos)
            continue;

        size_t after_ext = ext_pos + ext.length();
        char next_char;

        if (_req.uri.length() > after_ext)
            next_char = _req.uri[after_ext];
        else
            next_char = '\0';

        if (next_char != '\0' && next_char != '?' && next_char != '/')
            continue;
        
        data.hasCgi = true;
        data.CgiInterp = interp;
        data.cgi_extn = ext;
        data.script_path = _req.uri.substr(0, after_ext);

        if (next_char == '?')
        {
            if (has_query)
                data.query = _req.uri.substr(query_pos + 1);
        }
        else if (next_char == '/')
        {
            size_t path_info_start = after_ext;
            size_t path_info_end;

            if (has_query)
                path_info_end = query_pos;
            else
                path_info_end = _req.uri.length();

            data.PathInfo = _req.uri.substr(path_info_start, path_info_end - path_info_start);

            if (has_query)
                data.query = _req.uri.substr(query_pos + 1);
        }
        else
        {
            if (has_query)
                data.query = _req.uri.substr(query_pos + 1);
        }

        break;
    }
    // print all data members
    std::cerr << "\033[1;34m[CGI]\033[0m CGI Data:\033[0m" << std::endl;
    std::cerr << "\033[1;34m[CGI]\033[0m Query: " << data.query << std::endl;
    std::cerr << "\033[1;34m[CGI]\033[0m Script Path: " << data.script_path << std::endl;
    std::cerr << "\033[1;34m[CGI]\033[0m CGI Extension: " << data.cgi_extn << std::endl;
    std::cerr << "\033[1;34m[CGI]\033[0m CGI Interpreter: " << data.CgiInterp << std::endl;
    std::cerr << "\033[1;34m[CGI]\033[0m Path Info: " << data.PathInfo << std::endl;
    
    return data;
}




bool CgiHandler::IsCgi(void)
{
    return (_data.hasCgi);
}

std::string GetKey(std::map<std::string, std::string> map ,std::string target)
{
    std::string value;
        
    for (std::map<std::string, std::string>::const_iterator it = map.begin(); it != map.end(); ++it) 
    {
        std::cerr << "KEY: " << it->first << " , Value: " << it->second << std::endl;
        std::string current_key = it->first;
        std::transform(current_key.begin(), current_key.end(), current_key.begin(), ::tolower);
        if (current_key == target) 
        {
            value = it->second;
            break;
        }
    }
    return (value);
}


CgiState *CgiHandler::execCgi(Connection &conn)
{
    conn.updateLastActivity();
    std::cerr << "last activity time: " << conn.getLastActivity() << std::endl;
    CgiState *f = new CgiState();
    std::string cmd;
    std::string fp; // Full path to interpreter
    std::string str; // Script path to pass as argv[1]
    pid_t pid;
    char **env;

    std::string check_existing;

    check_existing = _data.script_path.substr(1);
    if (access((char *)check_existing.c_str(), F_OK) < 0)
    {
        delete f;
        conn.getCurrentRequest().throwHttpError(404, "Script file not found, path: {" + check_existing + "}");
        return NULL;
    }
    
    // Determine command to pass to execve
    if (_data.CgiInterp.find("/") != std::string::npos)
    cmd = _data.CgiInterp.substr(_data.CgiInterp.find_last_of("/") + 1); // e.g., "php-cgi" from "/usr/bin/php-cgi"
    else
    cmd = _data.CgiInterp; // e.g., "python3"
    
    fp  = full_path(_env_paths, _data.CgiInterp);
    if (fp.empty())
    {
        std::cerr << "Error: CGI interpreter not found: " << _data.CgiInterp << std::endl;
        delete f;
        conn.getCurrentRequest().throwHttpError(500, "Internal Server Error : interpreter not found.");
        return NULL;
    }
    
    str = _data.script_path.substr(1);
    
    int pfd[2]; // Pipe for CGI output
    int pfd_in[2]; // Pipe for CGI input (for POST requests)
    
    if (pipe(pfd) == -1) // Pipe for stdout from CGI
    {
        perror("pipe stdout failed");
        delete f;
        conn.getCurrentRequest().throwHttpError(500, "Internal Server Error");
        return NULL;
    }
    
    if (_req.method == POST && pipe(pfd_in) == -1) // Pipe for stdin to CGI (only for POST)
    {
        perror("pipe stdin failed");
        close(pfd[0]);
        close(pfd[1]);
        delete f;
        conn.getCurrentRequest().throwHttpError(500, "Internal Server Error");
        return NULL;
    }
    
    env = set_env(conn); // Environment variables prepared
    pid = fork();
    if (pid == -1) // Fork failed
    {
        perror("fork failed");
        close(pfd[0]); close(pfd[1]);
        if (_req.method == POST) 
        {
            close(pfd_in[0]); 
            close(pfd_in[1]); 
        }
        deleteEnvp(env);
        delete f;
        conn.getCurrentRequest().throwHttpError(500, "Internal Server Error");
        return NULL;
    }
    else if (pid == 0) // Child process (CGI)
    {
        std::cerr << "HELLO FROM CHILD PID: " << getpid() << std::endl;
        // Redirect stdout to parent
        dup2(pfd[1], STDOUT_FILENO);
        close(pfd[0]); // Close read end in child
        close(pfd[1]); // Close write end after duping

        // Redirect stdin from parent for POST requests
        if (_req.method == POST)
        {
            dup2(pfd_in[0], STDIN_FILENO);
            close(pfd_in[0]); // Close read end in child
            close(pfd_in[1]); // Close write end in child
        }
        else
        {
            // For GET or others, connect an empty pipe to stdin
            int null_fd[2];
            if (pipe(null_fd) == -1)
                exit(1); // Pipe failure

            dup2(null_fd[0], STDIN_FILENO); // Child will read EOF
            close(null_fd[0]);
            close(null_fd[1]);
        }       
                
        // **CRITICAL: Set up proper working directory and script path**
        // Convert URI path to filesystem path
        std::string filesystem_script_path = _data.script_path;
        std::cerr << "Full script path: " << filesystem_script_path << std::endl;
        
        // Get the directory containing the script
        size_t last_slash = filesystem_script_path.rfind('/');
        std::string script_dir_path = ".";
        std::string script_filename = filesystem_script_path;
        
        if (last_slash != std::string::npos) {
            script_dir_path = filesystem_script_path.substr(0, last_slash);
            script_filename = filesystem_script_path.substr(last_slash + 1);
        }
        
        script_dir_path = "." + script_dir_path; // Make it relative to current working directory 
        std::cerr << "script_dir_path: " << script_dir_path << std::endl;
        std::cerr << "script_filename: " << script_filename << std::endl;

        if (chdir(script_dir_path.c_str()) == -1) 
        {
            perror("chdir failed");
            exit(1); // Exit child process on chdir failure
        }
        // Execute the CGI script
        // argv[0] should be the interpreter name, argv[1] is the script filename
        char *final_argv[3];
        final_argv[0] = (char *)cmd.c_str(); // Just the interpreter name
        final_argv[1] = (char *)script_filename.c_str(); // Script filename relative to CWD
        final_argv[2] = NULL;
        
        execve(fp.c_str(), final_argv, env);
        perror("execve failed"); // Only reached if execve fails
        if (_req.method == POST)
            close(pfd_in[1]);
        exit(1); // Child must exit on execve failure
    }
    else // Parent process
    {
       close(pfd[1]); // Write end, not needed
        if (_req.method == POST)
            close(pfd_in[0]); // Read end, not needed

        fcntl(pfd[0], F_SETFL, O_NONBLOCK); // protcect
        if (_req.method == POST)
            fcntl(pfd_in[1], F_SETFL, O_NONBLOCK);

        f->output_fd = pfd[0];
        if (_req.method == POST)
            f->input_fd = pfd_in[1]; // Only for POST requests
        else
            f->input_fd = -1; // No input for GET requests
        f->pid = pid;
        f->bodySent = false;
        f->script_path = _data.script_path;
        f->startTime = std::time(0);
        f->connection = &conn; // Store the connection for later use
        deleteEnvp(env);
        return f;
    }
}