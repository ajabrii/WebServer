
# include "../includes/CgiState.hpp"
#include "../includes/Connection.hpp"
#include "../includes/Reactor.hpp"
#include "../includes/Utils.hpp"

CgiState::CgiState() : pid(-1), output_fd(-1), input_fd(-1),rawOutput("") ,startTime(0), headersParsed(false), bodySent(false), bytesWritten(0), script_path("") {}


CgiState::~CgiState()
{
    pid_t oldPid = -1;
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
    if (pid != -1)
    {
        oldPid = pid;
        waitpid(pid, NULL, WNOHANG);
        pid = -1;
    }
    std::cerr << "\033[1;34m[CGI]\033[0m CGI state cleaned up for PID: " << oldPid << std::endl;
}

void CgiState::writeToScript(Connection& conn)
{
    if (!this->bodySent && this->input_fd != -1)
    {
        const std::string& body = conn.getCurrentRequest().body;
        size_t totalSize = body.size();
        size_t remaining = totalSize - this->bytesWritten;
        if (remaining > 0)
        {
            ssize_t written = write(this->input_fd, body.c_str() + this->bytesWritten,remaining);

            if (written > 0)
            {
                this->bytesWritten += written;

                if (this->bytesWritten >= totalSize)
                {
                    close(this->input_fd);
                    this->input_fd = -1;
                    this->bodySent = true;
                    std::cout << "\033[1;34m[CGI]\033[0m Body fully sent to CGI script for fd: " << conn.getFd() << std::endl;
                }
            }
            else if (written == 0)
            {
                std::cerr << "[CGI] write() returned 0 bytes (unexpected)\n";
                close(this->input_fd);
                this->input_fd = -1;
                this->bodySent = true;
            }
            else
            {
                std::cerr << "[CGI] write() failed while sending body to CGI (fatal)\n";
                close(this->input_fd);
                this->input_fd = -1;
                this->bodySent = true;
            }
        }
        else
        {
            close(this->input_fd);
            this->input_fd = -1;
            this->bodySent = true;
            std::cout << "\033[1;34m[CGI]\033[0m Empty body (0 bytes) sent to CGI script for fd: " << conn.getFd() << std::endl;
        }
    }
}

void CgiState::readFromScript(Connection& conn , Reactor& reactor)
{
    char buffer[4096];
    ssize_t n = read(conn.getCgiState()->output_fd, buffer, sizeof(buffer));
    if (n > 0)
    {
        conn.getCgiState()->rawOutput.append(buffer, n);
    }
    else if (n == 0)
    {
        std::cout << "\033[1;34m[CGI]\033[0m CGI output EOF detected\n";

        std::cerr << "\033[1;34m[CGI]\033[0m Raw CGI output: " << conn.getCgiState()->rawOutput << std::endl;

        HttpResponse resp = parseCgiOutput(conn.getCgiState()->rawOutput);
        setConnectionHeaders(resp, conn.isKeepAlive());
        conn.writeData(resp.toString());

        conn.getCgiState()->connection->updateLastActivity();
        std::cerr << "CONN FD is : " << conn.getFd() << ", cgi connection fd is : " << conn.getCgiState()->connection->getFd() << std::endl;
        reactor.removeConnection(conn.getFd());
        std::cout << "\033[1;31m[-]\033[0m Connection closed (CGI done)" << std::endl;
    }
    else
    {
        perror("CGI read error");
        reactor.removeConnection(conn.getFd());
    }
}