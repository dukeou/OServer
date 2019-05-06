#ifndef O_HTTP_REQUEST
#define O_HTTP_REQUEST

typedef enum
{
    e_http_request_parse_status_begin,
    e_http_request_parse_status_method = e_http_request_parse_status_begin,
    e_http_request_parse_status_url,
    e_http_request_parse_status_version,
    e_http_request_parse_status_header,
    e_http_request_parse_status_error,
}http_request_parse_status_t;
typedef struct
{
    http_method_t method;
    http_url_t url;
    http_version_t version;
    http_header_collection_t headers;
    http_request_parse_status_t parse_status;
}http_request_t;
void http_request_init(http_request_t *req);
int http_request_reset(http_request_t *req);
int http_request_parse(http_request_t *req, const char *data, int size, int *rdlen, int *is_end);
void http_request_log(http_request_t *req);
#endif /* O_HTTP_REQUEST */
