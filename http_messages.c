#include "http_messages.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>

#define SPACE (" ")
#define CRLF ("\r\n")


/*
 * Return the reason string for a particular status code. You might find this
 * helpful when implementing response_string().
 */

const char *status_reason(int status) {
  switch (status) {
  case 100:
    return "Continue";
  case 101:
    return "Switching Protocols";
  case 200:
    return "OK";
  case 201:
    return "Created";
  case 202:
    return "Accepted";
  case 203:
    return "Non-Authoritative Information";
  case 204:
    return "No Content";
  case 205:
    return "Reset Content";
  case 206:
    return "Partial Content";
  case 300:
    return "Multiple Choices";
  case 301:
    return "Moved Permanently";
  case 302:
    return "Found";
  case 303:
    return "See Other";
  case 304:
    return "Not Modified";
  case 305:
    return "Use Proxy";
  case 307:
    return  "Temporary Redirect";
  case 400:
    return "Bad Request";
  case 401:
    return "Unauthorized";
  case 402:
    return "Payment Required";
  case 403:
    return "Forbidden";
  case 404:
    return "Not Found";
  case 405:
    return "Method Not Allowed";
  case 406:
    return "Not Acceptable";
  case 407:
    return "Proxy Authentication Required";
  case 408:
    return "Request Time-out";
  case 409:
    return "Conflict";
  case 410:
    return "Gone";
  case 411:
    return "Length Required";
  case 412:
    return "Precondition Failed";
  case 413:
    return "Request Entity Too Large";
  case 414:
    return "Request-URI Too Large";
  case 415:
    return  "Unsupported Media Type";
  case 416:
    return "Requested range not satisfiable";
  case 417:
    return "Expectation Failed";
  case 500:
    return "Internal Server Error";
  case 501:
    return "Not Implemented";
  case 502:
    return "Bad Gateway";
  case 503:
    return "Service Unavailable";
  case 504:
    return "Gateway Time-out";
  case 505:
    return "HTTP Version not supported";
  default:
    return "Unknown status";
  }
} /* status_reason() */

/*
char *get_header_value(const http_request *request, char *key) {
  for (int i = 0; i < request->num_headers; i++) {
    if (!strcmp(request->headers[i].key, key)) {
      return request->headers[i].value;
    }
  }
  return NULL;
}
*/

/*
 * This function is used to append a string to another
 */

char *append(char *str, char *appen, int len1, int len2) {
  str = realloc(str, len1 + len2);
  strcat(str, appen);
  return str;
} /* append() */



char *append_headers(char *str, http_response *resp) {
  header *headers = resp->headers;
  for (int i = 0; i < resp->num_headers; i++) {
    str = append(str, headers[i].key, strlen(str), strlen(headers[i].key));
    str = append(str, ":", strlen(str), 1);
    str = append(str, SPACE, strlen(str), strlen(SPACE));
    str = append(str, headers[i].value, strlen(str), strlen(headers[i].value));
    str = append(str, CRLF, strlen(str), strlen(CRLF));
  }
  str = append(str, CRLF, strlen(str), strlen(CRLF));
  return str;
}

int get_response_length(http_response *resp) {
  char *cont_len = get_header_value_response(resp, "Content-Length");
  int data_len = 0;
  if (cont_len != NULL) {
    data_len = atoi(cont_len);
  }

  char *data = NULL;
  if (resp->message_body != NULL) {
    data = strdup(resp->message_body);
    free(resp->message_body);
    resp->message_body = NULL;
  }

  char *str = response_string(resp);

  if (data != NULL) {
    resp->message_body = strdup(data);
    free(data);
    data = NULL;
  }

  return data_len + strlen(str);
}

/*
 * Create the actual response string to be sent over the socket, based
 * on the parameter.
 */

char *response_string(http_response *resp) {
  // TODO: Replace this code and correctly create the HTTP response from the
  // argument

  char *str = malloc(100);
  sprintf(str, " %d", resp->status_code);
  str = append(str, SPACE, strlen(str), strlen(SPACE));
  str = append(str, resp->reason_phrase, strlen(str),
      strlen(resp->reason_phrase));
  str = append(str, CRLF, strlen(str), strlen(CRLF));

  str = append_headers(str, resp);

  /*
  char *incorrect = "Connection: close\r\n"
    "Content-Length: 12\r\n"
    "\r\n"
    "Hello CS252!\r\n";
*/

  char *data = resp->message_body;

  int other_len = strlen(str);
  char *cont_len = get_header_value_response(resp, "Content-Length");
  int data_len = 0;
  if (cont_len != NULL) {
    data_len = atoi(cont_len);
  }

  if (data != NULL) {
    str = append(str, data, other_len, data_len);
  }

  char *to_string = malloc(sizeof(char) * (other_len +
                                           data_len +
                                           strlen(resp->http_version) +
                                           1));

  strcpy(to_string, resp->http_version);
  strcat(to_string, str);

  printf("{%s}\n", to_string);

  return to_string;
} /* response_string() */


/*
 * This function is used to get the value of the header key.
 */

char *get_header_value(http_request *request, char *key) {
  for (int i = 0; i < request->num_headers; i++) {
    if (!strcmp(request->headers[i].key, key)) {
      return request->headers[i].value;
    }
  }
  return NULL;
} /* get_header_value() */


char *get_header_value_response(http_response *resp, char *key) {
  for (int i = 0; i < resp->num_headers; i++) {
    if (!strcmp(resp->headers[i].key, key)) {
      return resp->headers[i].value;
    }
  }
  return NULL;

}

/*
 * This function is used to add a header to a response.
 */

void add_header_to_response(http_response *response, header *head) {
  response->num_headers++;
  response->headers = realloc(response->headers,
      response->num_headers * sizeof(header));
  response->headers[response->num_headers - 1] = *head;
} /* add_header_to_response() */


/*
 * This function is used to find a substring of a buf.
 */

char *substring(char *buf, int start, int end) {
  char *new_str = (char *) malloc(end - start + 1);
  for (int i = 0; i < end - start; i++) {
    new_str[i] = buf[start + i];
  }
  new_str[end - start] = '\0';
  return new_str;
} /* substring */


/*
 * Print the request to stdout, useful for debugging.
 */

void print_request(http_request *request) {
  // Magic string to help with autograder

  printf("\\\\==////REQ\\\\\\\\==////\n");

  printf("Method: {%s}\n", request->method);
  printf("Request URI: {%s}\n", request->request_uri);
  printf("Query string: {%s}\n", request->query);
  printf("HTTP Version: {%s}\n", request->http_version);

  printf("Headers: \n");
  for (int i = 0; i < request->num_headers; i++) {
    printf("field-name: %s; field-value: %s\n",
           request->headers[i].key, request->headers[i].value);
  }

  printf("Message body length: %ld\n", strlen(request->message_body));
  printf("%s\n", request->message_body);

  // Magic string to help with autograder

  printf("//==\\\\\\\\REQ////==\\\\\n");
} /* print_request() */
