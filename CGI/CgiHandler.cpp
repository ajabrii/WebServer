#include "CgiHandler.hpp"

 CgiHandler::CgiHandler(const std::string& scriptPath,
                const std::map<std::string, std::string>& env,
                const std::string& inputBody)
{
    
}

CgiHandler::~CgiHandler()
{
    ;
}

bool CgiHandler::start()
{
   ;
}                // fork + execve
bool CgiHandler::isComplete()
{
        ;
}          // check if child finished
std::string CgiHandler::getOutput()
{
    ;
}    // read output from pipe
void CgiHandler::killIfTimeout()
{
    ;
}       // optional timeout handling
pid_t CgiHandler::getPid() const
{
    ;
}