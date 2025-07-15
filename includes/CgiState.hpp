#pragma once

#include <string>

class CgiState
{
    public:
    pid_t pid;
    int output_fd;
    int input_fd; // only for POST
    std::string rawOutput;
    time_t startTime;
    bool headersParsed = false;
        // Optional:
    std::string headerBuffer;
    std::string bodyBuffer;
    std::string script_path;
    std::string pendingBody;
};
