#include "Webserver.hpp"


Server_block::Server_block()
{
    host = "";
    port = 0;
    client_max_body_size = 1024 * 1024; // Default to 1MB
    // server_names.clear();
    // error_pages.clear();
    // routes.clear();
}

