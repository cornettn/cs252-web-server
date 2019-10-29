#include "http_messages.h"

/*
 * Handle a CGI request
 */

http_response handle_cgi_bin(const http_request *request) {
  http_response resp = { 0 };
  resp.http_version = request->http_version;

  // TODO: Implement this

  return resp;
} /* handle_cgi_bin() */
