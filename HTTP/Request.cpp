#include "Request.hpp"

std::string trim(const std::string& str)
{
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos)
        return "";

    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

HttpRequest parseHttpRequest(const std::string& rawRequest) {
    HttpRequest req;
    size_t headerEnd = rawRequest.find("\r\n\r\n");

    std::string headerSection = rawRequest.substr(0, headerEnd);
    std::string bodySection = rawRequest.substr(headerEnd + 4);

    std::istringstream stream(headerSection);
    std::string line;

    // First line: method, path, version
    std::getline(stream, line);
    std::istringstream requestLine(line);
    requestLine >> req.method >> req.path >> req.version;

    // Remaining lines: headers
    while (std::getline(stream, line)) {
        if (line.back() == '\r') 
            line.pop_back(); // remove \r

        size_t colon = line.find(":");
        if (colon != std::string::npos) {
            std::string key = line.substr(0, colon);
            std::string value = line.substr(colon + 1);
            value = trim(value);
            req.headers[key] = value;
        }
    }

    req.body = bodySection;
    return req;
}

