#include "http_messages.h"

/*
 * Default handler for file not found (404)
 */

http_response handle_default(const http_request *request) {
    http_response resp = { 0 };
    resp.http_version = request->http_version;
    resp.status_code = 404;
    char *msg = (char *) status_reason(resp.status_code);
    resp.reason_phrase = msg;
    return resp;
} /* handle_default() */
