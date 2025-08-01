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
    const char* hexChars = "0123456789ABCDEF";
    unsigned long connVal = reinterpret_cast<unsigned long>(conn);

    for (int i = 0; i < 16; ++i)
    {
        int seed = rand() + requestCounter + static_cast<int>(connVal % 9973) + i;
        oss << hexChars[seed % 16];
    }

    return oss.str();
}
