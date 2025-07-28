#include "../includes/SessionID.hpp"
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <unistd.h>

std::string SessionID::generate(Connection* conn, int requestCounter) 
{
    std::ostringstream oss;
    for (int i = 0; i < 16; ++i) 
    {
        int r = rand() ^ ((reinterpret_cast<unsigned long>(conn) >> i) & 0xF) ^ (requestCounter & 0xF);
        oss << "0123456789ABCDEF"[r % 16];
    }
    return oss.str();
}
