
# include "../includes/CgiState.hpp"
#include "../includes/Connection.hpp"
#include "../includes/Reactor.hpp"
#include "../includes/Utils.hpp"

void CgiState::writeToScript(Connection& conn)
{
    std::cerr << "in fd  : " << this->input_fd << " bodySent: " << this->bodySent << " bytesWritten: " << this->bytesWritten << std::endl;
    if (!this->bodySent && this->input_fd != -1)
    {
        const std::string& body = conn.getCurrentRequest().body;
        size_t totalSize = body.size();
        size_t remaining = totalSize - this->bytesWritten;
        std::cerr << ", remaining bytes: " << remaining << std::endl;
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
                // Write returned 0 bytes — unexpected on pipes
                std::cerr << "[CGI] write() returned 0 bytes (unexpected)\n";
                close(this->input_fd);
                this->input_fd = -1;
                this->bodySent = true;
            }
            else // written == -1
            {
                // Don't know errno (can't check EAGAIN), assume fatal
                std::cerr << "[CGI] write() failed while sending body to CGI (fatal)\n";
                close(this->input_fd);
                this->input_fd = -1;
                this->bodySent = true;
            }
        }
        else
        {
            // Empty body case — Content-Length: 0
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
        // for (std::map<int, Connection*>::iterator it = reactor.getConnectionMap().begin(); it != reactor.getConnectionMap().end(); ++it) 
        // {
        //  std::cerr << "Connection fd: " << it->first << " at address: " << it->second << std::endl;
        // }

        HttpResponse resp = parseCgiOutput(conn.getCgiState()->rawOutput);
        setConnectionHeaders(resp, conn.isKeepAlive());
        conn.writeData(resp.toString());

        conn.getCgiState()->connection->updateLastActivity();
        //print fd of conn and cgi->connection
        std::cerr << "CONN FD is : " << conn.getFd() << ", cgi connection fd is : " << conn.getCgiState()->connection->getFd() << std::endl;
        // print all connectionMap of reactor <int, Connection *>  , id and address \n in c++98, using iterator
        reactor.removeConnection(conn.getFd()); // 8 
        // delete cgiState;
        // conn.setCgiState(NULL);
        // reactor.removeConnection(conn.getCgiState()->output_fd);
        // cleanupCgi(conn);
        std::cout << "\033[1;31m[-]\033[0m Connection closed (CGI done)" << std::endl;
    } 
    else 
    {
        perror("CGI read error");
        // reactor.removeConnection(conn.getCgiState()->output_fd);
        // cleanupCgi(conn);
        reactor.removeConnection(conn.getFd());
    }
}