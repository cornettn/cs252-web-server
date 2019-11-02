#include "http_messages.h"

#include <stdio.h>

#define REFERER ("Referer")
/*
 * You should implement this function and use it in server.c
 */

http_response handle_htdocs(const http_request *request) {
    http_response resp = { 0 };
    resp.http_version = request->http_version;

    // char *referer = get_header_value(request, REFERER);
    // printf("%s\n", referer);

    // TODO: Get the request URL, verify the file exists, and serve it

    return resp;
}

