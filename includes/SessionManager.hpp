/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SessionManager.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/02 09:56:15 by ajabri            #+#    #+#             */
/*   Updated: 2025/08/02 09:56:16 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SESSION_MANAGER_HPP
#define SESSION_MANAGER_HPP

#include <string>
#include <map>

class SessionManager
{
    private:
        std::string sessionDir;

    public:
        SessionManager();
        SessionManager(const std::string& directory);
        void save(const std::string& session_id, const std::map<std::string, std::string>& data);
        std::map<std::string, std::string> load(const std::string& session_id);
        std::string buildSessionFilePath(const std::string& session_id) const;
};

#endif
