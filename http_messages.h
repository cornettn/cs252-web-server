#ifndef HTTP_MESSAGES_H
#define HTTP_MESSAGES_H

#define ROOT ("./http-root-dir/htdocs")

const char *status_reason(int status);

typedef struct {
  char *key;
  char *value;
} header;

typedef struct {
  char *method;
  char *request_uri;
  char *http_version;
  int num_headers;
  header *headers;
  char *message_body;
  char *query;
} http_request;

typedef struct {
  char *http_version;
  int status_code;
  char *reason_phrase;
  int num_headers;
  header *headers;
  char *message_body;
} http_response;


void print_request(http_request *request);
void add_header_to_response(http_response *response, header *head);
char *get_header_value(http_request *request, char *key);
char *get_header_value_response(http_response *resp, char *key);
char *substring(char *buf, int start, int end);
char *response_string(http_response *response);
int get_response_length(http_response *resp);

http_response handle_request(const http_request *req, int status);

#endif // HTTP_MESSAGES_H
