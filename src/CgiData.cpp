#include "../includes/CgiData.hpp"

CgiData::CgiData() : query(""), script_path(""), cgi_extn(""), PathInfo("") ,hasCgi(false)
{

}

void CgiData::DebugPrint(void)
{

    std::cerr << "CgiExt: " << this->cgi_extn  << std::endl
    << "hasCgi: " << this->hasCgi  << std::endl
    << "PathInfo: " << this->PathInfo  << std::endl
    << "query: " << this->query  << std::endl
    << "script_path: " << this->script_path  << std::endl;
}
