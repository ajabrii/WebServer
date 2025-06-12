#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#include <string>
#include <map>

class CgiHandler 
{
    public:
        CgiHandler(const std::string& scriptPath,
                const std::map<std::string, std::string>& env,
                const std::string& inputBody);

        ~CgiHandler();

        bool start();                // fork + execve
        bool isComplete();          // check if child finished
        std::string getOutput();    // read output from pipe
        void killIfTimeout();       // optional timeout handling
        pid_t getPid() const;

    private:
        std::string _scriptPath;
        std::map<std::string, std::string> _env;
        std::string _inputBody;

        int _stdinPipe[2];
        int _stdoutPipe[2];
        pid_t _pid;
        bool _complete;
        time_t _startTime;

        void closePipes();
        char** buildEnvp();
        char** buildArgv();
};

#endif
