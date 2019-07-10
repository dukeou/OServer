#ifndef O_HTTP_URL
#define O_HTTP_URL
typedef struct url_query_item
{
    struct http_url_query_item *prev;
    struct http_url_query_item *next;
    char *name;
    int name_len;
    char *value;
    int value_len;
}http_url_query_item_t;
typedef struct
{
    list_t query_list;
    char *data;
    int data_len;
}http_url_query_info_t;
typedef struct http_url_path_seg
{
    struct http_url_path_seg *prev;
    struct http_url_path_seg *next;
    char *data;
    int data_len;
}http_url_path_seg_t;
typedef struct url_path_info
{
    o_stack_t path_stack;
    char *data;
    int data_len;
}http_url_path_info_t;
typedef struct
{
    char *host;
    int host_len;
    char *port;
    int port_len;
    http_url_path_info_t path;
    http_url_query_info_t query;
    char url[1024];
}http_url_t;
char http_hex_char_to_num(unsigned char x);
int http_url_unescape(char *url);
char *http_url_escape(char *url);
void http_url_init(http_url_t *url);
void http_url_destory(http_url_t *url);
int http_url_parse(const char *data, int size, int *parse_len, http_url_t *url);
#endif /* O_HTTP_URL */
