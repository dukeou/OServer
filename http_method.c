#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "http_method.h"

static const char *methods[] = 
{
    "GET ",
    "PUT ",
    "DELETE ",
    "PATCH ",
    "POST ",
    "HEAD ",
    "OPTIONS ",
};

int http_method_parse(const char *data, int size, int *parse_len, http_method_t *m)
{
    const char *p, *t, *end;
    char ok;
    http_method_t x;
    *parse_len = 0;
    if(!data || size <= 0)
        return -1;
    ok = 1;
    t = data;
    end = data + size;
    switch(data[0])
    {
        case 'G':
            x = e_http_get;
            p = methods[x] + 1;
            ++t;
            break;
        case 'P':
            if(data[1] == 'U')
            {
                x = e_http_put;
                p = methods[x] + 2;
                t += 2;
            }
            else if(data[1] == 'O')
            {
                x = e_http_post;
                p = methods[x] + 2;
                t += 2;
            }
            else
            {
                return -1;
            }
            break;
        case 'H':
            x = e_http_head;
            p = methods[x] + 1;
            ++t;
            break;
        case 'O':
            x = e_http_options;
            p = methods[x] + 1;
            ++t;
            break;
        case 0:
            return 0;
        default:
            return -1;
    }
    while((t < end) && *p && ok)
        ok &= (*t++ == *p++);
    if(ok && !(*p))
    {
        *parse_len = t - data;
        *m = x;
        return 0;
    }
    if(ok && t == end)
        return 0;
    return -1;
}
const char *http_method_str(http_method_t m)
{
    return methods[m];
}
