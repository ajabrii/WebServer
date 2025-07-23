#pragma once

#include <string>
#include <sys/wait.h>
#include <iostream>

typedef class Connection Connection;
class CgiState
{
    public:
        CgiState() : pid(-1), output_fd(-1), input_fd(-1),rawOutput("") ,startTime(0), headersParsed(false), bodySent(false), bytesWritten(0), script_path("") {};
        ~CgiState() 
        {
            std::cerr << "\n\033[1;34m[CGI]\033[0m Cleaning up CGI state for PID: " << pid << std::endl;
            if (output_fd != -1)
            {
                close(output_fd);
                output_fd = -1;
            }
            if (input_fd != -1)
            {
                close(input_fd);
                input_fd = -1;
            }
            std::cerr << "\033[1;34m[CGI]\033[0m CGI WAITS OR NOT: " << pid << std::endl;
            if (pid != -1)
            {
                waitpid(pid, NULL, 0);
                pid = -1;
            }
            std::cerr << "\033[1;34m[CGI]\033[0m CGI state cleaned up for PID: " << pid << std::endl;
        };
        pid_t pid;
        int output_fd;
        int input_fd; // only for POST
        std::string rawOutput;
        time_t startTime;
        bool headersParsed;
        bool bodySent;
        size_t bytesWritten;
            // Optional:
        std::string script_path;
        Connection* connection;
};
