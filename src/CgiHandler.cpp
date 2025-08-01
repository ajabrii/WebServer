/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: baouragh <baouragh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/01 19:44:18 by baouragh          #+#    #+#             */
/*   Updated: 2025/08/01 23:21:27 by baouragh         ###   ########.fr       */
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
        env_map["SERVER_PORT"] = "80";
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


std::vector<std::string> split_string(const std::string& path, char c)
{
    std::vector<std::string> subs;
    size_t start_pose = 0;
    size_t c_pose = 0;
    size_t length_path = 0;

    c_pose = path.find(c, start_pose);
    while (c_pose != std::string::npos)
    {
        length_path = c_pose - start_pose;
        subs.push_back(path.substr(start_pose, length_path));
        start_pose = c_pose + 1;
        c_pose = path.find(c, start_pose);
    }
    subs.push_back(path.substr(start_pose));
    return subs;
}

std::string full_path(std::string paths, std::string cmd)
{
    if (!access(cmd.c_str(), X_OK))
            return (cmd);
    std::vector<std::string> subs =  split_string(paths, ':');
    
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
    return data;
}

bool CgiHandler::IsCgi(void)
{
    return (_data.hasCgi);
}

CgiState *CgiHandler::execCgi(Connection &conn)
{
    conn.updateLastActivity();
    CgiState *f = new CgiState();
    std::string cmd;
    std::string fullPath;
    std::string str;
    pid_t pid;
    char **env;

    if (_data.script_path.empty() || _data.script_path[0] != '/' || _data.script_path.find("..") != std::string::npos)
    {
        delete f;
        conn.getCurrentRequest().throwHttpError(400, "Invalid script path.");
        return NULL;
    }

    std::string check_existing = _data.script_path.substr(1);
    if (access(check_existing.c_str(), F_OK) < 0)
    {
        delete f;
        conn.getCurrentRequest().throwHttpError(404, "Script file not found, path: {" + check_existing + "}");
        return NULL;
    }

    if (_data.CgiInterp.find("/") != std::string::npos)
        cmd = _data.CgiInterp.substr(_data.CgiInterp.find_last_of("/") + 1);
    else
        cmd = _data.CgiInterp;

    fullPath = full_path(_env_paths, _data.CgiInterp);
    if (fullPath.empty())
    {
        delete f;
        conn.getCurrentRequest().throwHttpError(500, "Internal Server Error : interpreter not found.");
        return NULL;
    }

    str = _data.script_path.substr(1);

    int pfd[2];
    int pfd_in[2];

    if (pipe(pfd) == -1)
    {
        delete f;
        conn.getCurrentRequest().throwHttpError(500, "Internal Server Error");
        return NULL;
    }

    if (_req.method == POST && pipe(pfd_in) == -1)
    {
        close(pfd[0]);
        close(pfd[1]);
        delete f;
        conn.getCurrentRequest().throwHttpError(500, "Internal Server Error");
        return NULL;
    }

    env = set_env(conn);
    pid = fork();
    if (pid == -1)
    {
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
    else if (pid == 0)
    {
        dup2(pfd[1], STDOUT_FILENO);
        close(pfd[0]);
        close(pfd[1]);

        if (_req.method == POST)
        {
            dup2(pfd_in[0], STDIN_FILENO);
            close(pfd_in[0]);
            close(pfd_in[1]);
        }
        else
        {
            int null_fd = open("/dev/null", O_RDONLY);
            if (null_fd == -1) _exit(1);
            dup2(null_fd, STDIN_FILENO);
            close(null_fd);
        }

        std::string filesystem_script_path = _data.script_path;
        size_t last_slash = filesystem_script_path.rfind('/');
        std::string script_dir_path = ".";
        std::string script_filename = filesystem_script_path;

        if (last_slash != std::string::npos)
        {
            script_dir_path = filesystem_script_path.substr(0, last_slash);
            script_filename = filesystem_script_path.substr(last_slash + 1);
        }

        if (chdir(script_dir_path.c_str()) == -1)
        {
            deleteEnvp(env);
            _exit(1);
        }

        char *final_argv[3];
        final_argv[0] = (char *)cmd.c_str();
        final_argv[1] = (char *)script_filename.c_str();
        final_argv[2] = NULL;

        execve(fullPath.c_str(), final_argv, env);
        deleteEnvp(env);
        _exit(1);
    }
    else
    {
        close(pfd[1]);
        if (_req.method == POST)
            close(pfd_in[0]);

        if (fcntl(pfd[0], F_SETFL, O_NONBLOCK) == -1)
        {
            delete f;
            deleteEnvp(env);
            conn.getCurrentRequest().throwHttpError(500, "Internal Server Error");
            return NULL;
        }

        if (_req.method == POST && fcntl(pfd_in[1], F_SETFL, O_NONBLOCK) == -1)
        {
            close(pfd[0]);
            delete f;
            deleteEnvp(env);
            conn.getCurrentRequest().throwHttpError(500, "Internal Server Error");
            return NULL;
        }

        f->output_fd = pfd[0];
        if (_req.method == POST)
            f->input_fd = pfd_in[1];
        else
            f->input_fd = -1;
        f->pid = pid;
        f->bodySent = false;
        f->script_path = _data.script_path;
        f->startTime = std::time(0);
        f->connection = &conn;
        deleteEnvp(env);
        return f;
    }
}
