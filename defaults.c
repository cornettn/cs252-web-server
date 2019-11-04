#include "http_messages.h"

#include <string.h>

#include "routes.h"

/*
 * Default handler for file not found (404)
 */

http_response handle_default(const http_request *request) {
  http_response resp = { 0 };
  resp.http_version = request->http_version;
  resp.status_code = 404;
  char *msg = (char *) status_reason(resp.status_code);
  resp.reason_phrase = msg;
  resp.message_body = strdup("404 File Not Found");
  return resp;
} /* handle_default() */


http_response handle_forbidden(const http_request *request) {
  http_response resp = {0};
  resp.http_version = request->http_version;
  resp.status_code = 403;
  resp.reason_phrase = (char *) status_reason(resp.status_code);
  resp.message_body = "Forbidden File";
  return resp;
}


http_response handle_request(const http_request *request, int status_code) {
  http_response resp = {0};
  resp.http_version = request->http_version;
  resp.status_code = status_code;
  resp.reason_phrase = (char *) status_reason(status_code);
  resp.message_body = "Uh Oh";
  return resp;
}
