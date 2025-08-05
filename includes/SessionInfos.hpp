/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SessionInfos.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/02 09:56:10 by ajabri            #+#    #+#             */
/*   Updated: 2025/08/02 09:56:11 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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


        public:
            SessionInfos();
            ~SessionInfos();
            const std::map<std::string, std::string>& getSessionData() const;
            std::map<std::string, std::string>& getCookies();
            const std::string& getSessionPath() const;
            const std::string& getSessionId() const;

            void setSessionData(const std::map<std::string, std::string>& data);
            void setCookies(const std::map<std::string, std::string>& cookies);
            void setSessionPath(void);
            void setSessionId(const std::string& id);
};

