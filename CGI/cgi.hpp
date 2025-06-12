#ifndef CGI_HPP
#define CGI_HPP

#include "../HTTP/Request.hpp"
#include <sys/wait.h>

void cgi_runer(Request &req);

#endif