#ifndef SESSION_ID_HPP
#define SESSION_ID_HPP

#include <string>

class SessionID 
{
    public:
        static std::string SessionID::generate(Connection* conn, int requestCounter);
};

#endif
