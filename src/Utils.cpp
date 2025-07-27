/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: baouragh <baouragh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/14 12:00:00 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/27 18:28:20 by baouragh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Utils.hpp"
#include "../includes/ResponseBuilder.hpp"
#include "../includes/SessionManager.hpp"
#include "../includes/SessionID.hpp"
#include "../includes/CookieParser.hpp"
#include "../includes/Connection.hpp"
#include "../includes/HttpRequest.hpp"
#include <string>
#include <map>
#include <algorithm>



std::string clean_line(std::string line)
{
    line.erase(0, line.find_first_not_of(" \t\n\r"));
    line.erase(line.find_last_not_of(" \t\n\r") + 1);

    if (!line.empty() && line[line.size() - 1] == ';')
    {
        line.erase(line.size() - 1);
    }
    return line;
}

bool shouldKeepAlive(const HttpRequest &request)
{
    std::string connection = request.GetHeader("connection");
    std::transform(connection.begin(), connection.end(), connection.begin(), ::tolower); // http is case-insensitive

    if (connection == "close")
        return false;
    return true;
}

void setConnectionHeaders(HttpResponse &response, bool keepAlive)
{
    if (keepAlive)
    {
        response.headers["Connection"] = "keep-alive";
        response.headers["Keep-Alive"] = "timeout=60, max=100";
    }
    else
    {
        response.headers["Connection"] = "close";
    }
}

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

std::string buildSetCookieHeader(const std::string& session_id) 
{
    return "Set-Cookie: session_id=" + session_id + "; Path=/; HttpOnly\r\n";
}


void HandleCookies(Connection& conn, HttpRequest& req)
{
    SessionManager sessionManager;
    SessionInfos& sessionInfos = conn.getSessionInfos();
        
    if (req.headers.find("cookie") != req.headers.end()) 
    {
        std::string cookieHeader = req.headers["cookie"];
        std::map<std::string, std::string> cookies = CookieParser::parse(cookieHeader);

        sessionInfos.setCookies(cookies);

        if (cookies.find("session_id") != cookies.end())
        {
            // std::cout << "\033[1;33m[Session]\033[0m Session ID found in cookies" << std::endl;
            std::string sessionId = cookies["session_id"];
                
            if (access(sessionManager.buildSessionFilePath(sessionId).c_str(), F_OK) == 0)
            {
                // std::cout << "\033[1;33m[Session]\033[0m Session ID exists: " << sessionId << std::endl;
                ;
            }
            else
            {
                // std::cout << "\033[1;33m[Session]\033[0m Session ID does not exist, creating new session" << std::endl;
                    // Create a new session file
                sessionManager.save(sessionId, sessionInfos.getCookies());
            }
        }
        else
        {
            // std::cout << "\033[1;33m[Session]\033[0m No session ID found in cookies" << std::endl;
            // Generate a new session ID if not present
            std::string newSessionId = SessionID::generate(&conn, conn.getRequestCount());
            sessionInfos.getCookies()["session_id"] = newSessionId;
                
            if (access(sessionManager.buildSessionFilePath(newSessionId).c_str(), F_OK) == 0)
            {
                // std::cout << "\033[1;33m[Session]\033[0m Session ID exists: " << newSessionId << std::endl;
                ;
            }
            else
            {
                // std::cout << "\033[1;33m[Session]\033[0m Session ID does not exist, creating new session" << std::endl;
                    // Create a new session file
                sessionManager.save(newSessionId, sessionInfos.getCookies());
            }
        }
    } // if not set-cookie
    else 
    {
        std::cout << "\033[1;33m[Session]\033[0m No cookies found in request" << std::endl;
        // Generate a new session ID if no cookies are present
        std::string newSessionId = SessionID::generate(&conn, conn.getRequestCount());
        sessionInfos.getCookies()["session_id"] = newSessionId;

        if (access(sessionManager.buildSessionFilePath(newSessionId).c_str(), F_OK) == 0)
        {
                // std::cout << "\033[1;33m[Session]\033[0m Session ID exists: " << newSessionId << std::endl;
                ;
        }
        else
        {
                // std::cout << "\033[1;33m[Session]\033[0m Session ID does not exist, creating new session" << std::endl;
                // Create a new session file
                sessionManager.save(newSessionId, sessionInfos.getCookies());
        }
    }
    sessionInfos.setSessionPath(sessionManager.buildSessionFilePath(sessionInfos.getSessionId()));
    sessionInfos.setSessionId(sessionInfos.getCookies()["session_id"]);
    conn.getCurrentRequest().SessionId = conn.getSessionInfos().getSessionId();
    sessionInfos.setSessionData(sessionManager.load(sessionInfos.getSessionId()));
        
}

void HandleTimeOut( std::vector<Connection*>& connections, Reactor &reactor)
{
    for (size_t j = 0; j < connections.size(); ++j) 
    {
        Connection* conn = connections[j];
        if (conn->isTimedOut()) 
        {
            std::cerr << "\033[1;31m[!]\033[0m Connection timed out: " << conn->getFd() << std::endl;
            // send 408 Request Timeout response
            HttpResponse timeoutResponse;
            timeoutResponse.statusCode = 408;
            timeoutResponse.statusText = "Request Timeout";
            timeoutResponse.body = "Your request has timed out due to inactivity.";
            timeoutResponse.headers["Content-Type"] = "text/plain";
            timeoutResponse.headers["Content-Length"] = Utils::toString(timeoutResponse.body.size());
            setConnectionHeaders(timeoutResponse, conn->isKeepAlive());
            timeoutResponse.SetCookieHeaders(*conn);
                
            reactor.removeConnection(conn->getFd());
            delete conn; // Clean up connection object
            connections.erase(connections.begin() + j);
            --j; // Adjust index after removal
        }
        else if (conn->getFd() != -1)
        {
            ;
                // curent vs last activity print and KEEP_ALIVE_TIMEOUT - last activity and limit of timeout
                // std::cout << "Connection fd: " << conn->getFd()
                //           << ", Keep-alive: " << (conn->isKeepAlive() ? "Yes" : "No") <<
                //           ", Time still to time out: " << (KEEP_ALIVE_TIMEOUT - (time(NULL) - conn->getLastActivity())) << " seconds" << std::endl;
        }
        else
        {
            delete conn; // Clean up invalid connection object
            connections.erase(connections.begin() + j);
            --j; // Adjust index after removal
        }
    }
}

// ============================================

void handleNewConnection(Reactor &reactor, const Event &event)
{
    HttpServer* server = reactor.getServerByListeningFd(event.fd);
    std::cout << "\033[1;32m[+]\033[0m New connection detected on fd: " << event.fd << std::endl;
    if (server)
    {
        Connection* conn = new Connection(server->acceptConnection(event.fd));
        conn->updateLastActivity();
        reactor.addConnection(conn, server);
        std::cout << NEW_CLIENT_CON << std::endl;
        bool keepAlive = shouldKeepAlive(conn->getCurrentRequest());      
        if (conn->getRequestCount() >= REQUEST_LIMIT_PER_CONNECTION)
            keepAlive = false;
        conn->setKeepAlive(keepAlive);
    }
}