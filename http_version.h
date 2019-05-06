#ifndef O_HTTP_VERSION
#define O_HTTP_VERSION
typedef enum
{
    e_http_version_1_0,
    e_http_version_1_1,
    e_http_version_none,
}http_version_t;
int http_version_parse(const char *data, int size, int *parsed_len, http_version_t *v);
int http_version_len(http_version_t ver);
int http_version_to_str(http_version_t ver, char *buf, int size);
#endif /* O_HTTP_VERSION */
