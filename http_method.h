#ifndef O_HTTP_METHOD
#define O_HTTP_METHOD
typedef enum
{
    e_http_get,
    e_http_put,
    e_http_delete,
    e_http_patch,
    e_http_post,
    e_http_head,
    e_http_options,
    e_http_none,
}http_method_t;
int http_method_parse(const char *data, int size, int *parsed_len, http_method_t *m);
const char *http_method_str(http_method_t m);
#endif /* O_HTTP_METHOD */
