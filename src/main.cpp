/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: baouragh <baouragh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/11 13:36:53 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/23 18:28:46 by baouragh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/ConfigInterpreter.hpp"
#include "../includes/HttpServer.hpp"
#include "../includes/Reactor.hpp"
#include "../includes/Router.hpp"
#include "../includes/RequestDispatcher.hpp"
#include "../includes/CgiHandler.hpp"
#include "../includes/Errors.hpp"
#include "../includes/Utils.hpp"
#include <sstream>
#include <algorithm>
#include <cctype>
#include <signal.h>
#include <cstdlib>
# define REQUEST_LIMIT_PER_CONNECTION 100

static volatile bool g_shutdown = false;
static std::vector<HttpServer*>* g_servers = NULL;
static Reactor* g_reactor = NULL;

void signalHandler(int signum) {
    (void)signum;
    g_shutdown = true;
}

// Helper: Convert string to lowercase
std::string toLower(const std::string& s) {
    std::string result = s;
    for (size_t i = 0; i < result.size(); ++i)
        result[i] = std::tolower(result[i]);
    return result;
}

// Helper: Trim leading spaces
void ltrim(std::string& s) {
    size_t i = 0;
    while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) ++i;
    s.erase(0, i);
}
HttpResponse parseCgiOutput(const std::string& raw) 
{
    HttpResponse response;

    size_t headerEnd = raw.find("\r\n\r\n");
    if (headerEnd == std::string::npos) 
    {
        response.statusCode = 500;
        response.statusText = "Internal Server Error";
        response.body = "CGI script did not return valid headers.";
        return response;
    }

    std::string headerPart = raw.substr(0, headerEnd);
    std::string bodyPart = raw.substr(headerEnd + 4);

    std::istringstream headerStream(headerPart);
    std::string line;
    bool statusParsed = false;

    // Read first line
    if (std::getline(headerStream, line)) 
    {
        if (!line.empty() && *line.rbegin() == '\r')
            line.erase(line.size() - 1);

        if (line.compare(0, 5, "HTTP/") == 0) 
        {
            std::istringstream statusStream(line);
            std::string httpVersion;
            statusStream >> httpVersion >> response.statusCode;
            std::getline(statusStream, response.statusText);
            ltrim(response.statusText);
            statusParsed = true;
        } 
        else 
        {
            // Reset headerStream to re-parse first line
            headerStream.clear(); // Reset error flags
            headerStream.str(headerPart); // Rewind to start
        }
    }

    // Parse headers
    while (std::getline(headerStream, line)) 
    {
        if (!line.empty() && *line.rbegin() == '\r')
            line.erase(line.size() - 1);

        size_t colon = line.find(':');
        if (colon != std::string::npos) 
        {
            std::string key = line.substr(0, colon);
            std::string value = line.substr(colon + 1);
            ltrim(value);

            // Lowercase for lookup only
            std::string lowerKey = toLower(key);
            if (lowerKey == "status") 
            {
                std::istringstream statusStream(value);
                statusStream >> response.statusCode;
                std::getline(statusStream, response.statusText);
                ltrim(response.statusText);
                statusParsed = true;
            } 
            else 
            {
                response.headers[key] = value;
            }
        }
    }

    if (!statusParsed) 
    {
        response.statusCode = 200;
        response.statusText = "OK";
    }

    response.body = bodyPart;

    if (response.headers.find("Content-Length") == response.headers.end()) 
    {
        std::ostringstream oss;
        oss << response.body.size();
        response.headers["Content-Length"] = oss.str();
    }

    if (response.headers.find("Content-Type") == response.headers.end()) 
    {
        response.headers["Content-Type"] = "text/html";
    }

    return response;
}

void handleErrorEvent(const Event& event)
{
    std::string errorMsg = "Connection error on fd " + Utils::toString(event.fd) + ": ";
    
    if (event.errorType & POLLHUP) {
        errorMsg += "Client disconnected (POLLHUP)";
    }
    if (event.errorType & POLLERR) {
        errorMsg += "Socket error (POLLERR)";
    }
    if (event.errorType & POLLNVAL) {
        errorMsg += "Invalid file descriptor (POLLNVAL)";
    }
    
    // Error::logs(errorMsg);
    
    // Remove the connection immediately - this will clean up resources
    if (g_reactor) {
        g_reactor->removeConnection(event.fd);
    }
}

int main(int ac, char **av, char **envp)
{
    if (ac != 2) {
        Error::logs("Usage: " + std::string(av[0]) + " <config_file>");
        return 1;
    }

    try
    {
        ConfigInterpreter parser;
        parser.getConfigData(av[1]);
        parser.parse();
        parser.checkValues();
        std::string cgiEnv = parser.getPathForCGI(envp);

        std::cout << "[✔] Config loaded" << std::endl;

        std::vector<ServerConfig> configs = parser.getServerConfigs();
        std::vector<HttpServer*> servers;

        // Setup signal handlers for graceful shutdown
        signal(SIGINT, signalHandler);
        signal(SIGTERM, signalHandler);
        g_servers = &servers;

        //* === Setup servers === (done 100%)
        for (size_t i = 0; i < configs.size(); ++i) {
            HttpServer* server = new HttpServer(configs[i]);
            server->setup();
            servers.push_back(server);
        }

        //* === Setup the multiplixing monitor aka reactor === (done 99.99%)
        Reactor reactor;
        g_reactor = &reactor;
        for (size_t i = 0; i < servers.size(); ++i)
            reactor.registerServer(*servers[i]);

        std::vector<Connection*> connections;
        //* === Event loop ===
        while (!g_shutdown)
        {
            try {
                reactor.poll(); //? hna kan3mer wa7d struct smitha Event ghatl9awha f reactor class
                
                // Cleanup timed-out connections periodically
                reactor.cleanupTimedOutConnections();
                
                std::vector<Event> events = reactor.getReadyEvents(); //? Hna kangeti dik struct li fiha evensts li 3mrathom poll (kernel li 3merhom poll it's system call you for more infos go to reactor.cpp > void Reactor::poll())

                //? hna kanlopi ela ga3 events struct kola wahd o chno khasni ndir lih/bih isnewconnection isReadble (POLLIN) isWritble
                for (size_t i = 0; i < events.size(); ++i)
            {
                Event event = events[i];

                // Handle error events FIRST (highest priority)
                if (event.isNewConnection)
                {
                    std::cout << "\033[1;32m[+]\033[0m New connection detected on fd: " << event.fd << std::endl;
                    HttpServer* server = reactor.getServerByListeningFd(event.fd); //? hna kanchof ina server t connecta m3ah l client bach nchof ina route khsni nmchi lih mn ber3d
                    if (server)
                    {
                        Connection* conn = new Connection(server->acceptConnection(event.fd)); //? hna kan 9ad wahd object dial connection kan constructih b client object li kay creah (acceptih) server
                        conn->updateLastActivity(); // Initialize activity timestamp for new connection
                        reactor.addConnection(conn, server);
                        std::cout << NEW_CLIENT_CON << std::endl;
                        // add the new connection to the connections vector
                        connections.push_back(conn);
                    }
                }
                else if (event.isReadable || event.isPullHUP)
                {
                    if(event.isReadable)
                        std::cout << "\033[1;33m[>]\033[0m Readable event on fd: " << event.fd << std::endl;
                    if (event.isPullHUP)
                    {
                        std::cerr << "[POLLHUP] Connection closed: " << event.fd << std::endl;
                    }
                    Connection &conn = reactor.getConnection(event.fd);
                    CgiState *cgiState = conn.getCgiState();
                    if (cgiState)
                    {
                        std::cerr << "in fd  : " << cgiState->input_fd << " bodySent: " << cgiState->bodySent << " bytesWritten: " << cgiState->bytesWritten << std::endl;
                        if (!cgiState->bodySent && cgiState->input_fd != -1)
                        {
                                const std::string& body = conn.getCurrentRequest().body;
                                size_t totalSize = body.size();
                                size_t remaining = totalSize - cgiState->bytesWritten;
                                std::cerr << ", remaining bytes: " << remaining << std::endl;
                                if (remaining > 0)
                                {
                                    ssize_t written = write(cgiState->input_fd, body.c_str() + cgiState->bytesWritten,remaining);

                                    if (written > 0)
                                    {
                                        cgiState->bytesWritten += written;

                                        if (cgiState->bytesWritten >= totalSize)
                                        {
                                            close(cgiState->input_fd);
                                            cgiState->input_fd = -1;
                                            cgiState->bodySent = true;
                                            std::cout << "\033[1;34m[CGI]\033[0m Body fully sent to CGI script for fd: " << event.fd << std::endl;
                                        }
                                    }
                                    else if (written == 0)
                                    {
                                        // Write returned 0 bytes — unexpected on pipes
                                        std::cerr << "[CGI] write() returned 0 bytes (unexpected)\n";
                                        close(cgiState->input_fd);
                                        cgiState->input_fd = -1;
                                        cgiState->bodySent = true;
                                    }
                                    else // written == -1
                                    {
                                        // Don't know errno (can't check EAGAIN), assume fatal
                                        std::cerr << "[CGI] write() failed while sending body to CGI (fatal)\n";
                                        close(cgiState->input_fd);
                                        cgiState->input_fd = -1;
                                        cgiState->bodySent = true;
                                    }
                                }
                                else
                                {
                                    // Empty body case — Content-Length: 0
                                    close(cgiState->input_fd);
                                    cgiState->input_fd = -1;
                                    cgiState->bodySent = true;
                                    std::cout << "\033[1;34m[CGI]\033[0m Empty body (0 bytes) sent to CGI script for fd: " << event.fd << std::endl;
                                }
                        }
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
                            //     std::cerr << "Connection fd: " << it->first << " at address: " << it->second << std::endl;
                            // }

                            HttpResponse resp = parseCgiOutput(conn.getCgiState()->rawOutput);
                            conn.writeData(resp.toString());

                            conn.getCgiState()->connection->updateLastActivity();
                            //print fd of conn and cgi->connection
                            std::cerr << "CONN FD is : " << conn.getFd() << ", cgi connection fd is : " << conn.getCgiState()->connection->getFd() << std::endl;
                            // print all connectionMap of reactor <int, Connection *>  , id and address \n in c++98, using iterator

                            reactor.removeConnection(event.fd); // 8 
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
                            reactor.removeConnection(event.fd);
                        }
                    }
                    else
                    {
                        
                        HttpServer* server = reactor.getServerForClient(event.fd);
                        if (!server) {
                            reactor.removeConnection(event.fd);
                            std::cerr << "Error: No server found for client fd: " << event.fd << std::endl;
                            continue;
                        }
                        
                        try {
                            conn.readData(server);
                        } catch (const std::exception& e) {
                            std::cerr << "Connection read error: " << e.what() << std::endl;
                            reactor.removeConnection(event.fd);
                            continue;
                        }
    
                       if (conn.isRequestComplete()) 
                       {
                            
                            // std::cout << "dekhel" << std::endl;
                            std::cout << "\033[1;36m[>] Full Request Received and Parsed:\033[0m\n";
                            
    
                            HttpRequest& req = conn.getCurrentRequest();
    
                                // std::cout << "Method: " << req.method << std::endl;
                                // std::cout << "URI: " << req.uri << std::endl;
                                // std::cout << "Version: " << req.version << std::endl;
                                // if (!req.body.empty()) {
                                //     std::cout << "Body Length: " << req.body.length() << std::endl;
                                    // std::cout << "Body (first 100 chars): " << req.body.substr(0, 100) << "..." << std::endl;
                                // }
                                std::cout << RECEV_COMPLETE << std::endl;
                                
                        try
                        {
                            // HttpServer* server = reactor.getServerForClient(event.fd);
                            // HttpRequest& req = conn.getCurrentRequest();
                            //  std::cout << "Method: " << req.method << std::endl;
                            //     std::cout << "URI: " << req.uri << std::endl;
                            //     std::cout << "Version: " << req.version << std::endl;
                            //     if (!req.body.empty()) {
                            //         std::cout << "Body Length: " << req.body.length() << std::endl;
                            //         std::cout << "Body (first 100 chars): " << req.body.substr(0, 100) << "..." << std::endl;
                            //     }
                            Router router;
                            const RouteConfig* route = router.match(req, server->getConfig());
                            HttpResponse resp;
                                
                            // Print all request details here
                            // std::cout << "----- FULL REQUEST DETAILS -----" << std::endl;
                            // std::cout << "Method: " << req.method << std::endl;
                            // std::cout << "URI: " << req.uri << std::endl;
                            // std::cout << "Version: " << req.version << std::endl;
                            // std::cout << "Headers:" << std::endl;
                            // for (std::map<std::string, std::string>::const_iterator it = req.headers.begin(); it != req.headers.end(); ++it) {
                            //     std::cout << "  " << it->first << ": " << it->second << std::endl;
                            // }
                            // if (!req.body.empty()) {
                            //     std::cout << "Body Length: " << req.body.length() << std::endl;
                            //     std::cout << "Body: " << req.body << std::endl;
                            // }
                            
                            // exit(0);
                            if (route) 
                            {
                                CgiHandler cgi(*server, req, *route, event.fd, cgiEnv);
                                // try {
                                //     if (cgi.IsCgi())
                                //         handleCgi(conn);
                                // }
                                // catch (const std::runtime_error& e) {
                                //     HttpResponse errResp;
                                //     errResp.statusCode = e.statusCode();
                                //     errResp.statusText = getStatusText(e.statusCode()); // e.g. 403 -> "Forbidden"
                                //     errResp.body = e.what();
                                //     conn.writeData(errResp.toString());
                                //     return;
                                // }
                                if (cgi.IsCgi())
                                {
                                    conn.setCgiState(cgi.execCgi(conn));
                                    // print fd of conn
                                    std::cerr << "CGI fd: " << conn.getFd() << std::endl;

                                    if (conn.getCgiState()) 
                                    {
                                        // If CGI is running, watch its output
                                        reactor.watchCgi(&conn);
                                        std::cerr << "CONN FD is : " << conn.getFd() << ", cgi state address is : " << conn.getCgiState() << "\n";
                                        std::cout << "\033[1;34m[CGI]\033[0m Executing CGI script: " << conn.getCgiState()->script_path << std::endl;
                                        continue; // Skip response handling, wait for CGI output
                                    } 
                                    else 
                                    {
                                        std::cerr << "\033[1;31m[CGI]\033[0m Failed to execute CGI "<< std::endl;
                                        // Handle CGI execution failure
                                        // CGI execution failed
                                        resp.version = "HTTP/1.1";  // Fix: Set HTTP version
                                        resp.statusCode = 500;
                                        resp.statusText = "Internal Server Error";
                                        resp.headers["content-type"] = "text/html";
                                        resp.body = Error::loadErrorPage(500, server->getConfig());
                                        
                                        // C++98 compatible string conversion
                                        std::stringstream ss;
                                        ss << resp.body.size();
                                        resp.headers["content-length"] = ss.str();
                                    }
                                }
                                else
                                {
                                    RequestDispatcher dispatcher;
                                    resp = dispatcher.dispatch(req, *route, server->getConfig());
                                }
                            } 
                            else {
                                // No route found - return 404
                                std::cout << "::::::::::::::::::::::::::::::::::NOT-FOUND:::::::::::::::::::::::::::::::::::::" << std::endl;
                                resp.version = "HTTP/1.1";  // Fix: Set HTTP version
                                resp.statusCode = 404;
                                resp.statusText = "Not Found";
                                resp.headers["content-type"] = "text/html";
                                resp.body = Error::loadErrorPage(404, server->getConfig());
                                
                                // C++98 compatible string conversion
                                std::stringstream ss;
                                ss << resp.body.size();
                                resp.headers["content-length"] = ss.str();
                                
                                std::cout << "\033[1;33m[Main]\033[0m No route found for URI: " << req.uri << " - returning 404" << std::endl;
                            }
    
                            // Determine if we should keep the connection alive
                            bool keepAlive = shouldKeepAlive(req);
                            
                            // Limit number of requests per connection to prevent resource exhaustion
                            if (conn.getRequestCount() >= REQUEST_LIMIT_PER_CONNECTION)
                                keepAlive = false;
                            // Set appropriate connection headers
                            setConnectionHeaders(resp, keepAlive);
                            
                            // Send the response
                            conn.writeData(resp.toString());
                            conn.reset(); //m7i lkhra mn connection bach nwjdo request lakhra la kant connection keep alive
                            conn.updateLastActivity(); // Update activity timestamp after sending response
                            
                            // Handle connection based on keep-alive decision
                            if (keepAlive) {
                                conn.setKeepAlive(true);
                                conn.incrementRequestCount();
                                conn.resetForNextRequest();
                                conn.updateLastActivity(); // Reset timeout for keep-alive connection
                                std::cout << "\033[1;32m[+]\033[0m Connection kept alive (request #" << conn.getRequestCount() << ")" << std::endl;
                            } else {
                                reactor.removeConnection(event.fd);
                                std::cout << "\033[1;31m[-]\033[0m Connection closed" << std::endl;
                            }
                            }
                        catch (const std::runtime_error& e) {
                            std::string msg = e.what();
                            if (msg.find("incomplete body") != std::string::npos) {
                                continue;
                            } else {
                                std::cerr << "Parse error: " << msg << std::endl;
                                HttpResponse errorResp;
                                errorResp.version = "HTTP/1.1";
                                errorResp.statusCode = 400;
                                errorResp.statusText = "Bad Request";
                                errorResp.headers["content-type"] = "text/html";
                                errorResp.body = Error::loadErrorPage(400, server->getConfig());
                                
                        //         // C++98 compatible string conversion
                                std::stringstream ss;
                                ss << errorResp.body.size();
                                errorResp.headers["content-length"] = ss.str();
                                
                        //         // Always close connection on parse errors
                                setConnectionHeaders(errorResp, false);
                                conn.writeData(errorResp.toString());
                                conn.updateLastActivity(); // Update activity timestamp after error response
                                reactor.removeConnection(event.fd);
                            }
                        } catch (const std::exception& e) {
                            Error::logs("Connection error: " + std::string(e.what()));
                            reactor.removeConnection(event.fd);
                        }
                        } // End of if (conn.isRequestComplete())
                    }
                }
                else if (event.isError)
                {
                    std::cerr << "\033[1;31m[!]\033[0m Error event on fd: " << event.fd << std::endl;
                    handleErrorEvent(event);
                }
                
                
            } // End of for loop
            for (size_t j = 0; j < connections.size(); ++j) {
                    Connection* conn = connections[j];
                    if (conn->isTimedOut()) {
                        std::cerr << "\033[1;31m[!]\033[0m Connection timed out: " << conn->getFd() << std::endl;
                        // send 408 Request Timeout response
                        HttpResponse timeoutResponse;
                        timeoutResponse.statusCode = 408;
                        timeoutResponse.statusText = "Request Timeout";
                        timeoutResponse.body = "Your request has timed out due to inactivity.";
                        timeoutResponse.headers["Content-Type"] = "text/plain";
                        timeoutResponse.headers["Content-Length"] = Utils::toString(timeoutResponse.body.size());
                        conn->writeData(timeoutResponse.toString());
                        reactor.removeConnection(conn->getFd());
                        delete conn; // Clean up connection object
                        connections.erase(connections.begin() + j);
                        --j; // Adjust index after removal
                    }
                    else
                    {
                        // Update last activity for active connections
                        std::cout << "\033[1;32m[+]\033[0m Active connection: " << conn->getFd() << ", last activity: " << conn->getLastActivity() << std::endl;
                    }
                }
            } catch (const std::exception& e) {
                std::cerr << "Event loop error: " << e.what() << std::endl;
                // Continue with next iteration
            }
        } // End of while loop

        // === Graceful cleanup ===
        std::cout << "\n[INFO] Shutting down gracefully..." << std::endl;
        
        // Clean up all connections in reactor
        reactor.cleanup();
        
        // Clean up servers
        for (size_t i = 0; i < servers.size(); ++i) {
            delete servers[i];
        }
        servers.clear();

    } catch (const std::exception& e) {
        std::cerr << "Fatal: " << e.what() << std::endl;
        
        // Emergency cleanup

        if (g_servers) {
            for (size_t i = 0; i < g_servers->size(); ++i) {
                delete (*g_servers)[i];
            }
        }
        return 1;
    }

    return 0;
}
