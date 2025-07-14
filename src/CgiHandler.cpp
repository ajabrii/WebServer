/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: baouragh <baouragh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/01 19:44:18 by baouragh          #+#    #+#             */
/*   Updated: 2025/07/14 16:44:53 by baouragh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "../includes/CgiHandler.hpp"
# include "../includes/CgiData.hpp"
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
    // _serverSocket = server.getFd();
    _clientSocket = clientSocket;
    _data = check_cgi();
    _data.DebugPrint();
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
        std::strcpy(envp[i], entry.c_str());
        envp[i][entry.size()] = '\0';

        ++i;
    }
    envp[i] = NULL;
    // printEnvp(envp);
    return envp;
}


char **CgiHandler::set_env(void)
{
    std::map<std::string, std::string> env_map; // Use env_map for clarity
    std::string host;
    
    for (std::map<std::string, std::string>::const_iterator it = _req.headers.begin(); it != _req.headers.end(); ++it) 
    {
        std::string header_name = it->first;
        std::transform(header_name.begin(), header_name.end(), header_name.begin(), ::toupper); // Convert to uppercase
        std::replace(header_name.begin(), header_name.end(), '-', '_'); // Replace hyphens with underscores
        env_map[header_name] = it->second;
    }
    host = _req.GetHeader("host");

    env_map["REQUEST_METHOD"] = _req.method;
    env_map["PATH_INFO"] = _data.PathInfo;
    env_map["SCRIPT_NAME"] = _data.script_path;
    // Set SCRIPT_FILENAME to just the script filename after chdir
    std::string script_filename = _data.script_path;
    size_t last_slash = script_filename.rfind('/');
    if (last_slash != std::string::npos)
        script_filename = script_filename.substr(last_slash + 1);
    env_map["SCRIPT_FILENAME"] = script_filename;
    env_map["QUERY_STRING"] = _data.query;
    if (host.find(':') != std::string::npos)
    {
        env_map["SERVER_PORT"] = host.substr(host.find(":") + 1);
        env_map["SERVER_NAME"] = host.substr(0, host.find(":"));
    }
        
    // env_map["REMOTE_ADDR"] =; // Get client IP address // 
    env_map["GATEWAY_INTERFACE"] = "CGI/1.1";
    env_map["SERVER_PROTOCOL"] = "HTTP/1.1";
    // std::cout << "SERVER NAME: " << host.substr(0, host.find(":")) << ", PORT: " << host.substr(host.find(":") + 1) << std::endl; // REMOVE
    env_map["REDIRECT_STATUS"] = "200"; // Common for web servers using CGI
    
    if (_req.method == GET) 
    {
        env_map["CONTENT_LENGTH"] = "0";
    }
    else if (_req.method == POST) 
    {
        std::ostringstream oss_cl;
        oss_cl << _req.body.size();
        env_map["CONTENT_LENGTH"] = oss_cl.str();
        env_map["CONTENT_TYPE"] = _req.GetHeader("Content-Type");;
    }
    // for (std::map<std::string, std::string>::const_iterator it = _req.headers.begin(); it != _req.headers.end(); ++it)
    // {
    //     std::cout << "Key: " << it->first << ", Value: " << it->second << std::endl;
    // }
    // for (std::map<std::string, std::string>::const_iterator it = env_map.begin(); it != env_map.end(); ++it)
    // {
    //     std::cout << "Key: " << it->first << ", Value: " << it->second << std::endl;
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

    return ("");
}

CgiData CgiHandler::check_cgi(void) // cgi-bin/file.extns/path/path?var=value
{
    CgiData data;
    size_t exten_pose;
    std::string founded_extns;
    bool is_query = false;
    bool is_pathInfo = false;
    size_t query_pose;

    query_pose =  _req.uri.find('?');
    if (query_pose != std::string::npos)
        is_query = true;
    for (std::map<std::string, std::string>::const_iterator it = this->_route.cgi.begin(); it != this->_route.cgi.end(); ++it) // 
    {
        exten_pose = _req.uri.find(it->first);
        if (exten_pose == std::string::npos)
            continue;

        founded_extns = _req.uri.substr(exten_pose, it->first.size() + 1);

        if (founded_extns[it->first.size()] != '?' && founded_extns[it->first.size()] != '\0' && founded_extns[it->first.size()] != '/')
            continue;
        else if (founded_extns[it->first.size()] == '?')
        {
            founded_extns = founded_extns.substr(0, founded_extns.size() - 1);
            is_query = true;
        }
        else if (founded_extns[it->first.size()] == '/')
        {
            founded_extns = founded_extns.substr(0, founded_extns.size() - 1);
            if (query_pose == std::string::npos)
                data.PathInfo = _req.uri.substr(exten_pose + it->first.size(), query_pose - (exten_pose + it->first.size()));
            else
                data.PathInfo =  _req.uri.substr(exten_pose + founded_extns.size());
            is_pathInfo = true;
        }
        if (founded_extns == it->first || is_query || is_pathInfo)
        {
            data.hasCgi = true;
            data.CgiInterp = it->second;
            data.cgi_extn = founded_extns.substr(0, it->first.size()); 
            
            if (is_query)
                data.query = _req.uri.substr(query_pose);
            data.script_path =_req.uri.substr(0, exten_pose + data.cgi_extn.size());
        }
    }
    return (data);
}


bool CgiHandler::IsCgi(void)
{
    return (_data.hasCgi);
}

// HttpResponse CgiHandler::execCgi(void)
// {
//     HttpResponse f;//
//     std::string cmd;
//     std::string fp;
//     std::string str;
//     pid_t pid;
//     char **env;
//     env = set_env();
//     if (_data.CgiInterp.find("/") != std::string::npos)
//         cmd = _data.CgiInterp.substr(_data.CgiInterp.find_last_of("/") + 1);
//     else
//         cmd = _data.CgiInterp;
//     std::cout << "CMD: " << cmd << std::endl;
//     fp  = full_path(_env_paths, _data.CgiInterp); // TO DO HANDLE ERROR
//     std::cout << "FULL PATH: " << fp << std::endl;
//     // if (fp.empty())
//     //     return;
//     str = _data.script_path.substr(1);
//     char *argv[] = { (char *)cmd.c_str(), (char *)str.c_str() ,NULL };
//     int pfd[2];
//     if (pipe(pfd))
//         exit(1);
//     pid = fork();
//     if (!pid)
//     {
//         dup2(pfd[1], 1);
//         close(pfd[1]);
//         close(pfd[0]);
    
//         execve(fp.c_str(), argv, env);
//         perror("execve failed");
//         _exit(1);
//     }
//     // listen to the output and store it to pipe
//     else if (pid < 0)
//     {
//         std::cerr << "Fork failed" << std::endl;
//         close(pfd[0]);
//         close(pfd[1]);
//         // return;
//     }
//     close(pfd[1]);
    
//     std::string cgi_output_str;
//     char buffer[4096];
//     ssize_t bytes_read;
//     while ((bytes_read = read(pfd[0], buffer, 4096 - 1)) > 0) 
//     {
//         buffer[bytes_read] = '\0'; // Null-terminate the chunk
//         cgi_output_str.append(buffer); // Append to the output string
//     }
//     if (bytes_read == -1)
//     {
//         perror("read from pipe_stdout failed");
//     }
//     close(pfd[0]);
//     std::cout << "--------------------> " << cgi_output_str << std::endl;
//     f.version = "HTTP/1.1";
//     // f.headers["Content-Type"] = "text/html";
//     f.statusCode = 200;
//     f.body = cgi_output_str;
//     wait(NULL);
//     return f;   
// }

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


HttpResponse CgiHandler::execCgi(void)
{
    HttpResponse f;
    std::string cmd;
    std::string fp; // Full path to interpreter
    std::string str; // Script path to pass as argv[1]
    pid_t pid;
    char **env;

    std::string check_existing;

    check_existing = _data.script_path.substr(1);
    if (access((char *)check_existing.c_str(), F_OK) < 0)
    {
        f.statusCode = 404;
        f.statusText = "Internal Server Error";
        f.body = "CGI : Script file not found, path: {" + check_existing + "}" ;
        return f;
    }
    env = set_env(); // Environment variables prepared

    // Determine command to pass to execve
    if (_data.CgiInterp.find("/") != std::string::npos)
        cmd = _data.CgiInterp.substr(_data.CgiInterp.find_last_of("/") + 1); // e.g., "php-cgi" from "/usr/bin/php-cgi"
    else
        cmd = _data.CgiInterp; // e.g., "python3"

    fp  = full_path(_env_paths, _data.CgiInterp); // Get absolute path to interpreter
    // IMPORTANT: Handle fp.empty() case - it means interpreter not found
    if (fp.empty())
    {
        // Handle error: Interpreter not found (e.g., 500 Internal Server Error)
        // Free env memory
        for (int i = 0; env[i] != NULL; ++i) delete[] env[i];
        delete[] env;
        f.statusCode = 500;
        f.statusText = "Internal Server Error 1";
        f.body = "CGI Interpreter not found.";
        return f;
    }
    
    str = _data.script_path.substr(1);

    int pfd[2]; // Pipe for CGI output
    int pfd_in[2]; // Pipe for CGI input (for POST requests)

    if (pipe(pfd) == -1) // Pipe for stdout from CGI
    {
        perror("pipe stdout failed");
        // Clean up env
        for (int i = 0; env[i] != NULL; ++i) delete[] env[i];
        delete[] env;
        f.statusCode = 500;
        f.statusText = "Internal Server Error 2";
        f.body = "Server pipe creation failed.";
        return f;
    }

    if (_req.method == POST && pipe(pfd_in) == -1) // Pipe for stdin to CGI (only for POST)
    {
        perror("pipe stdin failed");
        close(pfd[0]);
        close(pfd[1]);
        // Clean up env
        for (int i = 0; env[i] != NULL; ++i) delete[] env[i];
        delete[] env;
        f.statusCode = 500;
        f.statusText = "Internal Server Error 3";
        f.body = "Server pipe creation failed.";
        return f;
    }

    pid = fork();
    if (pid == -1) // Fork failed
    {
        perror("fork failed");
        close(pfd[0]); close(pfd[1]);
        if (_req.method == POST) { close(pfd_in[0]); close(pfd_in[1]); }
        // Clean up env
        for (int i = 0; env[i] != NULL; ++i) delete[] env[i];
        delete[] env;
        f.statusCode = 500;
        f.statusText = "Internal Server Error 4";
        f.body = "Server fork failed.";
        return f;
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

        for (size_t i = 0; i < 3; i++)
        {
            std::cerr << "-------->FINAL ARGV[" << i << "]" << " " <<  final_argv[i] << std::endl;
        }
        
        
        execve(fp.c_str(), final_argv, env);
        perror("execve failed"); // Only reached if execve fails
        exit(1); // Child must exit on execve failure
    }
    else // Parent process
    {
        close(pfd[1]); // Close write end of output pipe in parent
        if (_req.method == POST)
        {
            close(pfd_in[0]); // Close read end of input pipe in parent
            // Write request body to CGI's stdin
            ssize_t bytes_written = write(pfd_in[1], _req.body.c_str(), _req.body.size());
            if (bytes_written == -1) {
                perror("write to pipe_stdin failed");
                // Handle error
            }
            close(pfd_in[1]); // Close write end after writing (sends EOF to CGI)
        }
        int status;
        int timeout_seconds = 5 ;
        bool child_finished = false;

        // Setup epoll for non-blocking read with timeout
        int epfd = epoll_create1(0);
        if (epfd == -1) 
        {
            perror("epoll_create1 failed");
            // Free env
            for (int i = 0; env[i] != NULL; ++i) delete[] env[i];
            delete[] env;
            f.statusCode = 500;
            f.statusText = "Internal Server Error (epoll)";
            f.body = "Server epoll creation failed.";
            return f;
        }
        struct epoll_event ev;
        ev.events = EPOLLIN;
        ev.data.fd = pfd[0];
        if (epoll_ctl(epfd, EPOLL_CTL_ADD, pfd[0], &ev) == -1) 
        {
            perror("epoll_ctl failed");
            close(epfd);
            for (int i = 0; env[i] != NULL; ++i) delete[] env[i];
            delete[] env;
            f.statusCode = 500;
            f.statusText = "Internal Server Error (epoll_ctl)";
            f.body = "Server epoll_ctl failed.";
            return f;
        }

        std::string cgi_output_str;
        char buffer[4096];
        ssize_t bytes_read;
        time_t start_time = time(NULL);

        while (true) 
        {
            int elapsed = time(NULL) - start_time;
            int remaining = timeout_seconds - elapsed;
            if (remaining <= 0) 
            {
                // Timeout reached
                kill(pid, SIGKILL);
                waitpid(pid, &status, 0);
                close(pfd[0]);
                close(epfd);
                for (int i = 0; env[i] != NULL; ++i) delete[] env[i];
                delete[] env;
                f.statusCode = 504;
                f.statusText = "Gateway Timeout 1";
                f.body = "CGI script execution timed out (epoll).";
                return f;
            }

            int nfds = epoll_wait(epfd, &ev, 1, remaining * 1000); // timeout in ms
            if (nfds == -1) 
            {
                perror("epoll_wait failed");
                break;
            }
            else if (nfds == 0) 
            {
                
                // Timeout reached (no data to read)
                std::cerr << "curr pid: " << getpid() << ",PID TO KILL: " << pid << std::endl;
                kill(pid, SIGKILL);
                waitpid(pid, &status, 0);
                close(pfd[0]);
                close(epfd);
                for (int i = 0; env[i] != NULL; ++i) delete[] env[i];
                delete[] env;
                f.statusCode = 504;
                f.statusText = "Gateway Timeout 2";
                f.body = "CGI script execution timed out (epoll wait).";
                return f;
            } 
            else 
            {
                // Data available to read
                bytes_read = read(pfd[0], buffer, sizeof(buffer) - 1);
                if (bytes_read > 0) 
                {
                    buffer[bytes_read] = '\0';
                    cgi_output_str.append(buffer);
                } 
                else if (bytes_read == 0)
                {
                    // EOF
                    child_finished = true;
                    break;
                } 
                else 
                {
                    perror("read from pipe_stdout failed\n");
                    break;
                }
            }
            // Also check if child finished
            pid_t result = waitpid(pid, &status, WNOHANG);

            std::cerr << "waitpid result: " << result << ", pid: " << pid << std::endl;
            if (result == pid) 
            {
                child_finished = true;
                break;
            }
        }

        close(pfd[0]);
        close(epfd);

        for (int i = 0; env[i] != NULL; ++i) delete[] env[i];
        delete[] env;

        if (!child_finished || (WIFEXITED(status) && WEXITSTATUS(status) != 0)) 
        {
            f.statusCode = 500;
            f.statusText = "Internal Server Error Child finish";
            f.body = "CGI script execution failed.";
            return f;
        }

        // Process CGI output as before...
        size_t header_end_pos = cgi_output_str.find("\r\n\r\n");
        if (header_end_pos == std::string::npos) 
        {
            f.statusCode = 500;
            f.statusText = "Internal Server Error 6";
            f.body = "CGI output malformed: No header end.";
            return f;
        }

        std::string cgi_headers = cgi_output_str.substr(0, header_end_pos);
        std::string cgi_body = cgi_output_str.substr(header_end_pos + 4); // +4 for \r\n\r\n

        std::istringstream iss(cgi_headers);
        std::string line;
        while (std::getline(iss, line) && !line.empty()) 
        {
            size_t colon_pos = line.find(':');
            if (colon_pos != std::string::npos) 
            {
                std::string name = line.substr(0, colon_pos);
                std::string value = line.substr(colon_pos + 1);
                value.erase(0, value.find_first_not_of(" \t"));
                value.erase(value.find_last_not_of(" \t\r\n") + 1);
                f.headers[name] = value;
            }
        }
        f.version = "HTTP/1.1";
        f.statusCode = 200;
        if (f.headers.count("Status")) 
        {
            std::string status_str = f.headers["Status"];
            size_t space_pos = status_str.find(' ');
            if (space_pos != std::string::npos) 
            {
                f.statusCode = std::atoi(status_str.substr(0, space_pos).c_str());
                f.statusText = status_str.substr(space_pos + 1);
            }
            f.headers.erase("Status");
        } 
        else 
        {
            f.statusText = "OK";
        }
        if (!f.headers.count("Content-Length")) 
        {
            f.headers["Content-Length"] = std::to_string(cgi_body.length());
        }
        f.body = cgi_body;
        return f;
    }
}
