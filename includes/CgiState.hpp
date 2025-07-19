#pragma once

#include <string>
#include <sys/wait.h>
#include <iostream>

class CgiState
{
    public:
        CgiState() : pid(-1), output_fd(-1), input_fd(-1), startTime(0), headersParsed(false) {};
        ~CgiState() 
        {
            std::cerr << "\n\033[1;34m[CGI]\033[0m Cleaning up CGI state for PID: " << pid << std::endl;
            if (output_fd != -1)
                close(output_fd);
            if (input_fd != -1)
                close(input_fd);
            std::cerr << "\033[1;34m[CGI]\033[0m CGI WAITS OR NOT: " << pid << std::endl;
            if (pid != -1)
                waitpid(pid, NULL, 0);
            std::cerr << "\033[1;34m[CGI]\033[0m CGI state cleaned up for PID: " << pid << std::endl;
        };
        pid_t pid;
        int output_fd;
        int input_fd; // only for POST
        std::string rawOutput;
        time_t startTime;
        bool headersParsed;
            // Optional:
        std::string headerBuffer;
        std::string bodyBuffer;
        std::string script_path;
        std::string pendingBody;
};
