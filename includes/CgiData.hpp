#pragma once

#include <iostream>

class CgiData
{
    public:
        std::string query;
        std::string script_path;
        std::string cgi_extn;
        std::string PathInfo; 
        bool hasCgi;

    public:
        CgiData();
        void DebugPrint(void);

};