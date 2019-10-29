#ifndef INCLUDE_ROUTES_HH_
#define INCLUDE_ROUTES_HH_

#include "http_messages.h"

http_response handle_cgi_bin(const http_request *request);
http_response handle_htdocs(const http_request *request);
http_response handle_default(const http_request *request);

typedef http_response (*handle) (const http_request *);

typedef struct{
    char *route_name;
    handle handle_func;
}  routes;

// A routes map, pairing string served and handle function
routes route_map[] = {
    {"/cgi-bin", &handle_cgi_bin},
    {"/", &handle_htdocs},
    {"", &handle_default}
};

#endif  // INCLUDE_ROUTES_HH_
