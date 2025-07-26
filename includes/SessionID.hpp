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
