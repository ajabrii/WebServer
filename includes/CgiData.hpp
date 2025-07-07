#pragma once

#include <iostream>

class CgiData
{
    public:
        std::string query;
        std::string script_path;
        std::string cgi_extn;
        std::string CgiInterp; // it may be a CMD or FULLPATH/CMD
        std::string PathInfo; 
        bool hasCgi;

    public:
        CgiData();
        void DebugPrint(void);

};