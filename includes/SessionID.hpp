/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SessionID.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/02 09:56:18 by ajabri            #+#    #+#             */
/*   Updated: 2025/08/02 09:56:19 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SESSION_ID_HPP
#define SESSION_ID_HPP

#include <string>

typedef class Connection Connection;
class SessionID
{
    private:
        SessionID() {}
    public:
        static std::string generate(Connection* conn, int requestCounter);
};

#endif
