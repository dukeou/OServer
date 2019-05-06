#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "btree.h"
#include "list.h"
#include "stack.h"
#include "logger.h"
#include "http_method.h"
#include "http_url.h"
#include "http_version.h"
#include "http_header.h"
#include "http_request.h"

void http_request_init(http_request_t *req)
{
    req->method = e_http_none;
    http_url_init(&req->url);
    req->version = e_http_version_none;
    req->parse_status = e_http_request_parse_status_begin;
    http_header_init(&req->headers);
}
int http_request_reset(http_request_t *req)
{
    req->method = e_http_none;
    http_url_destory(&req->url);
    req->version = e_http_version_none;
    req->parse_status = e_http_request_parse_status_begin;
    return http_header_destroy(&req->headers);
}
int http_request_parse(http_request_t *req, const char *data, int size, int *rdlen, int *is_end)
{
    int ret, parsed_len, len;
    http_request_parse_status_t last_status;
    const char *p;
    *rdlen = 0;
    *is_end = 0;
    p = data;
    len = size;
    l_og(e_log_level_debug, "http_request_parse enter\n");
    do
    {
        last_status = req->parse_status;
        switch(req->parse_status)
        {
            case e_http_request_parse_status_method:
                ret = http_method_parse(p, len, &parsed_len, &req->method);
                if(ret < 0)
                {
                    l_og(e_log_level_debug, "http_method_parse error\n");
                    req->parse_status = e_http_request_parse_status_error;
                }
                else if(parsed_len)
                {
                    l_og(e_log_level_debug, "http_method_parse parsed_len: %d\n", parsed_len);
                    req->parse_status = e_http_request_parse_status_url;
                }
                break;
            case e_http_request_parse_status_url:
                ret = http_url_parse(p, len, &parsed_len, &req->url);
                if(ret < 0)
                {
                    l_og(e_log_level_debug, "http_url_parse error\n");
                    req->parse_status = e_http_request_parse_status_error;
                }
                else if(parsed_len)
                {
                    l_og(e_log_level_debug, "http_url_parse parsed_len: %d\n", parsed_len);
                    req->parse_status = e_http_request_parse_status_version;
                }
                break;
            case e_http_request_parse_status_version:
                ret = http_version_parse(p, len, &parsed_len, &req->version);
                if(ret < 0)
                {
                    l_og(e_log_level_debug, "http_version_parse error\n");
                    req->parse_status = e_http_request_parse_status_error;
                }
                else if(parsed_len)
                {
                    l_og(e_log_level_debug, "http_version_parse parsed_len: %d\n", parsed_len);
                    req->parse_status = e_http_request_parse_status_header;
                }
                break;
            case e_http_request_parse_status_header:
                ret = http_header_parse(&req->headers, p, len, &parsed_len, is_end);
                if(ret < 0)
                {
                    l_og(e_log_level_debug, "http_header_parse error\n");
                    req->parse_status = e_http_request_parse_status_error;
                }
                break;
            default:
                req->parse_status = e_http_request_parse_status_error;
                break;
        }
        p += parsed_len;
        len -= parsed_len;
    }while(req->parse_status != e_http_request_parse_status_error && req->parse_status != last_status);
    if(req->parse_status == e_http_request_parse_status_error)
    {
        *is_end = 1;
        l_og(e_log_level_debug, "http_request_parse exit with error\n");
        return -1;
    }
    *rdlen = p - data;
    l_og(e_log_level_debug, "http_request_parse exit\n");
    return 0;
}
void http_request_log(http_request_t *req)
{
    int len, size, is_end;
    char *buf, *hptr;
    size = http_header_len(&req->headers);
    size += http_version_len(req->version);
    size += 2;
    buf = (char*)malloc(size);
    len = http_version_to_str(req->version, buf, size);
    buf[len] = '\0';
    hptr = buf + len + 1;
    len = http_header_to_str(&req->headers, hptr, buf + size - hptr, &is_end);
    hptr[len] = '\0';
    l_og(e_log_level_info, "\n%s%s %s\n%s\n\n", http_method_str(req->method), req->url.url, buf, hptr);
    free(buf);
}
