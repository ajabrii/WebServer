#pragma once

#include <map>
#include <string>
#include <iostream>

class SessionInfos
{
        private:
            std::map<std::string, std::string> sessionData;
            std::map<std::string, std::string> cookies;
            std::string SessionPath;
            std::string SessionId;
            std::string UploadDirectory;

        
        public:
            SessionInfos();
            ~SessionInfos();
            const std::map<std::string, std::string>& getSessionData() const;
            std::map<std::string, std::string>& getCookies();
            const std::string& getSessionPath() const;
            const std::string& getSessionId() const;
            const std::string& getUploadDirectory() const;

            void setSessionData(const std::map<std::string, std::string>& data);
            void setCookies(const std::map<std::string, std::string>& cookies);
            void setSessionPath(const std::string& path);
            void setSessionId(const std::string& id);
            void setUploadDirectory(const std::string& path);
};

