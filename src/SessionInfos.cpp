#include "../includes/SessionInfos.hpp"
SessionInfos::SessionInfos()
{
    UploadDirectory = "";
    SessionId = "";
}
SessionInfos::~SessionInfos()
{

}
const std::map<std::string, std::string>& SessionInfos::getSessionData() const
{
    return sessionData;
}
std::map<std::string, std::string>& SessionInfos::getCookies()
{
    return cookies;
}
const std::string& SessionInfos::getSessionPath() const
{
    return SessionPath;
}
const std::string& SessionInfos::getSessionId() const
{
    return SessionId;
}
const std::string& SessionInfos::getUploadDirectory() const
{
    return UploadDirectory;
}



void SessionInfos::setSessionData(const std::map<std::string, std::string>& data)
{
    sessionData = data;
}
void SessionInfos::setCookies(const std::map<std::string, std::string>& cookies)
{
    this->cookies = cookies;
}
void SessionInfos::setSessionPath(const std::string& path)
{
    (void)path; // Ignore the path parameter, we always use "./session/"
    SessionPath = "./session/" + SessionId;
}
void SessionInfos::setSessionId(const std::string& id)
{
    SessionId = id;
}
void SessionInfos::setUploadDirectory(const std::string& path)
{
    (void)path; 
    UploadDirectory = "./session";
}
