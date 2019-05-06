#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "logger.h"
#include "http_version.h"

static const char http_version_str[] = "HTTP/1.";
static const char http_line_delimiter[] = "\r\n";
int http_version_parse(const char *data, int size, int *parse_len, http_version_t *v)
{
    const char *p, *t, *end;
    char ok;
    if(!data || size <= 0)
        return 0;
    t = data;
    p = http_version_str;
    end = data + size;
    ok = 1;
    *parse_len = 0;
    while(t < end && *p && ok)
        ok &= (*t++ == *p++);
    if(t == end)
        return 0;
    if(ok && !(*p))
    {
        l_og(e_log_level_debug, "HTTP/1. matched 1\n");
        if(*t == '0' || *t == '1')
        {
            l_og(e_log_level_debug, "HTTP/1. matched 2\n");
            ++t;
            p = http_line_delimiter;
            while(t < end && *p && ok)
                ok &= (*t++ == *p++);
            if(ok && !(*p))
            {
                *parse_len = t - data;
                *v = (http_version_t)(e_http_version_1_0 + (*(t - 3) - '0'));
                return 0;
            }
            if(ok && *p)
                return 0;
        }
    }
    else if(ok && *p)
    {
        return 0;
    }
    return -1;
}
int http_version_len(http_version_t ver)
{
    return 9;
}
int http_version_to_str(http_version_t ver, char *buf, int size)
{
    int len;
    len = 9;
    switch(ver)
    {
        case e_http_version_1_0:
            strncpy(buf, "HTTP/1.0 ", size);
            break;
        case e_http_version_1_1:
        default:
            strncpy(buf, "HTTP/1.1 ", size);
            break;
    }
    return len > size ? size : len;
}
