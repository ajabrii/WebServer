#include "../includes/SessionInfos.hpp"
SessionInfos::SessionInfos()
{
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



void SessionInfos::setSessionData(const std::map<std::string, std::string>& data)
{
    sessionData = data;
}
void SessionInfos::setCookies(const std::map<std::string, std::string>& cookies)
{
    this->cookies = cookies;
}
void SessionInfos::setSessionPath(void)
{
    SessionPath = "./session/" + SessionId;
}
void SessionInfos::setSessionId(const std::string& id)
{
    SessionId = id;
}
