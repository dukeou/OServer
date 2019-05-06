#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>

#include "btree.h"
#include "logger.h"
#include "http_header.h"

static http_header_field_t http_header_fields[] = 
{
    {
        "Accept",
        6
    },
    {
        "Accept-Charset",
        14
    },
    {
        "Accept-Encoding",
        15
    },
    {
        "Accept-Language",
        15
    },
    {
        "Accept-Ranges",
        13
    },
    {
        "Access-Control-Allow-Credentials",
        32
    },
    {
        "Access-Control-Allow-Headers",
        28
    },
    {
        "Access-Control-Allow-Methods",
        28
    },
    {
        "Access-Control-Allow-Origin",
        27
    },
    {
        "Access-Control-Expose-Headers",
        29
    },
    {
        "Access-Control-Max-Age",
        22
    },
    {
        "Access-Control-Request-Headers",
        30
    },
    {
        "Access-Control-Request-Method",
        29
    },
    {
        "Age",
        3
    },
    {
        "Allow",
        5
    },
    {
        "Alt-Svc",
        7
    },
    {
        "Authorization",
        13
    },
    {
        "Cache-Control",
        13
    },
    {
        "Clear-Site-Data",
        15
    },
    {
        "Connection",
        10
    },
    {
        "Content-Disposition",
        19
    },
    {
        "Content-Encoding",
        16
    },
    {
        "Content-Language",
        16
    },
    {
        "Content-Length",
        14
    },
    {
        "Content-Location",
        16
    },
    {
        "Content-Range",
        13
    },
    {
        "Content-Security-Policy",
        23
    },
    {
        "Content-Security-Policy-Report-Only",
        35
    },
    {
        "Content-Type",
        12
    },
    {
        "Cookie",
        6
    },
    {
        "Cookie2",
        7
    },
    {
        "DNT",
        3
    },
    {
        "Date",
        4
    },
    {
        "ETag",
        4
    },
    {
        "Early-Data",
        10
    },
    {
        "Expect",
        6
    },
    {
        "Expect-CT",
        9
    },
    {
        "Expires",
        7
    },
    {
        "Feature-Policy",
        14
    },
    {
        "Forwarded",
        9
    },
    {
        "From",
        4
    },
    {
        "Host",
        4
    },
    {
        "If-Match",
        8
    },
    {
        "If-Modified-Since",
        17
    },
    {
        "If-None-Match",
        13
    },
    {
        "If-Range",
        8
    },
    {
        "If-Unmodified-Since",
        19
    },
    {
        "Index",
        5
    },
    {
        "Keep-Alive",
        10
    },
    {
        "Large-Allocation",
        16
    },
    {
        "Last-Modified",
        13
    },
    {
        "Location",
        8
    },
    {
        "Origin",
        6
    },
    {
        "Pragma",
        6
    },
    {
        "Proxy-Authenticate",
        18
    },
    {
        "Proxy-Authorization",
        19
    },
    {
        "Public-Key-Pins",
        15
    },
    {
        "Public-Key-Pins-Report-Only",
        27
    },
    {
        "Range",
        5
    },
    {
        "Referer",
        7
    },
    {
        "Referrer-Policy",
        15
    },
    {
        "Retry-After",
        11
    },
    {
        "Sec-WebSocket-Accept",
        20
    },
    {
        "Server",
        6
    },
    {
        "Server-Timing",
        13
    },
    {
        "Set-Cookie",
        10
    },
    {
        "Set-Cookie2",
        11
    },
    {
        "SourceMap",
        9
    },
    {
        "Strict-Transport-Security",
        25
    },
    {
        "TE",
        2
    },
    {
        "Timing-Allow-Origin",
        19
    },
    {
        "Tk",
        2
    },
    {
        "Trailer",
        7
    },
    {
        "Transfer-Encoding",
        17
    },
    {
        "Upgrade-Insecure-Requests",
        25
    },
    {
        "User-Agent",
        10
    },
    {
        "Vary",
        4
    },
    {
        "Via",
        3
    },
    {
        "WWW-Authenticate",
        16
    },
    {
        "Warning",
        7
    },
    {
        "X-Content-Type-Options",
        22
    },
    {
        "X-DNS-Prefetch-Control",
        22
    },
    {
        "X-Forwarded-For",
        15
    },
    {
        "X-Forwarded-Host",
        16
    },
    {
        "X-Forwarded-Proto",
        17
    },
    {
        "X-Frame-Options",
        15
    },
    {
        "X-XSS-Protection",
        16
    },
};
static int8_t http_header_fields_index[sizeof(http_header_fields)/sizeof(http_header_field_t)] = {0};
void http_header_init(http_header_collection_t *headers)
{
    btree_init(&headers->header_tree);
    headers->traverse_ptr = (http_header_t*)headers->header_tree.root;
}
void http_header_free_node(http_header_t *h, void *data)
{
    l_og(e_log_level_debug, "free http_header_t %p\n", h);
    free(h);
}
int http_header_destroy(http_header_collection_t *headers)
{
    if(headers->header_tree.root)
    {
        btree_post_order_traverse(&headers->header_tree, (visit_func)http_header_free_node, NULL);
        btree_init(&headers->header_tree);
        headers->traverse_ptr = (http_header_t*)headers->header_tree.root;
    }
    return 0;
}
int http_header_add(http_header_collection_t *headers, const char *name, int ln, const char *value, int lv)
{
    int id;
    if(!headers || !name || !value || !name[0] || !value[0])
        return -1;
    if((id = http_header_binary_n_search(name, ln)) == -1)
        return -1;
    return http_header_add_by_id(headers, id, value, lv);
}
int http_header_add_by_id(http_header_collection_t *headers, int id, const char *value, int lv)
{
    if(!headers || !value || !value[0])
        return -1;
    http_header_t *h = (http_header_t*)malloc(sizeof(http_header_t) + lv + 1);
    h->key = id;
    h->value = (char*)h + sizeof(http_header_t);
    h->len = lv;
    memcpy(h->value, value, lv);
    h->value[lv] = 0;
    l_og(e_log_level_debug, "tring to add %d to btree\n", id);
    btree_insert(&headers->header_tree, (bt_node_entry_t*)h);
    headers->traverse_ptr = (http_header_t*)headers->header_tree.root;
    return 0;
}
void http_header_accumulate_len(http_header_t *h, int *len)
{
    *len += http_header_fields[h->key].len + h->len + 4;
}
int http_header_len(http_header_collection_t *headers)
{
    int len = 2;
    btree_pre_order_traverse(&headers->header_tree, (visit_func)http_header_accumulate_len, &len);
    return len;
}
int http_header_accumelate_str(http_header_t *h, http_header_str_info_t *info)
{
    int len;
    if(!h)
    {
        l_og(e_log_level_debug, "header end \n");
        info->headers->traverse_ptr = NULL;
        return 1;
    }
    if(info->size >= (http_header_fields[h->key].len + h->len + 4))
    {
        len = snprintf(info->buf, info->size, "%s: %s\r\n", http_header_fields[h->key].str, h->value);
        info->size -= len;
        info->buf += len;
        info->headers->traverse_ptr = h;
        return 0;
    }
    l_og(e_log_level_debug, "header end no enough space\n");
    return 1;
}
int http_header_to_str(http_header_collection_t *headers, char *s, int size, int *is_end)
{
    http_header_str_info_t info;
    info.buf = s;
    info.size = size;
    info.headers = headers;
    btree_build_in_order_threading(&headers->header_tree);
    btree_thread_traverse((btree_t*)&headers->traverse_ptr, (visit_func2)http_header_accumelate_str, &info);
    if(NULL == headers->traverse_ptr && info.size >= 2)
    {
        l_og(e_log_level_debug, "add blank line\n");
        *(info.buf)++ = '\r';
        *(info.buf)++ = '\n';
        *is_end = 1;
        info.size -= 2;
        headers->traverse_ptr = (http_header_t*)headers->header_tree.root;
    }
    return size - info.size;
}
int http_header_parse(http_header_collection_t *headers, const char *buf, int len, int *rdlen, int *is_end)
{
    const char *s, *e, *ne, *c, *vs;
    char pre, err;
    s  = NULL;
    e  = NULL;
    ne = NULL;
    c  = NULL;
    vs = NULL;
    pre = 0;
    err = 0;
    *rdlen = 0;
    *is_end = 0;
    if(!buf || len <= 0)
        return 0;
    for(int j = 0; j < len && !err; ++j)
    {
        switch(buf[j])
        {
            case '\n':
                if(pre != '\r')
                {
                    return -1;
                }
                if(s)
                {
                    if(!vs || !e)
                    {
                        return -1;
                    }
                    if(e)
                        ++e;
                    else
                        e = vs + 1;
                    http_header_add(headers, s, ne - s, vs, e - vs);
                    *rdlen = j + 1;
                    s  = NULL;
                    e  = NULL;
                    ne = NULL;
                    c  = NULL;
                    vs = NULL;
                    pre = 0;
                }
                else
                {
                    *rdlen = j + 1;
                    *is_end = 1;
                    return 0;
                }
                break;
            case '\r':
                break;
            case ' ':
                if(s && !c)
                {
                    if(!ne)
                        ne = &buf[j];
                }
                break;
            case ':':
                if(!c)
                {
                    c = &buf[j];
                    if(!ne)
                        ne = c;
                }
                break;
            default:
                if(ne && !c)
                {
                    l_og(e_log_level_debug, "ne && !c\n");
                    l_og(e_log_level_debug, "ne = %s\n", ne);
                    return -1;
                }
                if(!s)
                {
                    s = &buf[j];
                }
                else if(c && !vs)
                {
                    vs = &buf[j];
                    e = vs;
                }
                else if(vs)
                {
                    e = &buf[j];
                }
                break;
        }
        pre = buf[j];
    }
    return 0;
}
void http_header_hash_all_fields()
{
    unsigned int i;
    for(i = 0; i < sizeof(http_header_fields)/sizeof(http_header_fields[0]); ++i)
    {
        http_header_fields[i].hash = http_header_hash(http_header_fields[i].str);
        http_header_fields_index[i] = i;
        l_og(e_log_level_debug, "%-35s : %04hX\n", http_header_fields[i].str, http_header_fields[i].hash);
    }
    http_header_fields_bubble_sort();
}
void http_header_fields_bubble_sort()
{
    unsigned int i, j, x;
    for(i = sizeof(http_header_fields)/sizeof(http_header_fields[0]) - 1; i > 0; --i)
    {
        for(j = 0; j < i; ++j)
        {
            if(http_header_fields[http_header_fields_index[j]].hash > http_header_fields[http_header_fields_index[j + 1]].hash)
            {
                x = http_header_fields_index[j] ^ http_header_fields_index[j + 1];
                http_header_fields_index[j] ^= x;
                http_header_fields_index[j + 1] ^= x;
            }
        }
    }
    for(i = 0; i < sizeof(http_header_fields)/sizeof(http_header_fields[0]); ++i)
    {
        l_og(e_log_level_debug, "%-35s : %hd\n", http_header_fields[http_header_fields_index[i]].str, http_header_fields[http_header_fields_index[i]].hash);
    }
}
int16_t http_header_hash(const char *str)
{
    int16_t hash;
    const char *p;
    int c;
    for(hash = 0, p = str; *p != 0; ++p)
    {
        c = tolower(*p);
        hash ^= c - 0x30;
        hash += c << 8;
    }
    return hash;
}
int16_t http_header_n_hash(const char *bin, int len)
{
    int c;
    int16_t hash;
    const char *p, *end;
    end = bin + len;
    for(hash = 0, p = bin; p < end; ++p)
    {
        c = tolower(*p);
        hash ^= c - 0x30;
        hash += c << 8;
    }
    return hash;
}
int http_header_binary_search(const char *target)
{
    int16_t targ_hash;
    int low, high, mid;
    targ_hash = http_header_hash(target);
    l_og(e_log_level_debug, "http_header_binary_search hash: %hd\n", targ_hash);
    low = 0; high = sizeof(http_header_fields_index)/sizeof(http_header_fields_index[0]) - 1;
    while(low <= high)
    {
        mid = (low + high) >> 1;
        l_og(e_log_level_debug, "http_header_binary_search mid: %d hash: %hd\n", mid, http_header_fields[http_header_fields_index[mid]].hash);
        if(targ_hash == http_header_fields[http_header_fields_index[mid]].hash)
        {
            if(strcasecmp(target, http_header_fields[http_header_fields_index[mid]].str) == 0)
                return http_header_fields_index[mid];
            else
                break;
        }
        else if(targ_hash < http_header_fields[http_header_fields_index[mid]].hash)
            high = mid - 1;
        else
            low = mid + 1;
    }
    return -1;
}
int http_header_binary_n_search(const char *target, int len)
{
    int16_t targ_hash;
    int low, high, mid;
    targ_hash = http_header_n_hash(target, len);
    l_og(e_log_level_debug, "http_header_binary_n_search hash: %hd\n", targ_hash);
    low = 0; high = sizeof(http_header_fields_index)/sizeof(http_header_fields_index[0]) - 1;
    while(low <= high)
    {
        mid = (low + high) >> 1;
        l_og(e_log_level_debug, "http_header_binary_n_search mid: %d hash: %hd\n", mid, http_header_fields[http_header_fields_index[mid]].hash);
        if(targ_hash == http_header_fields[http_header_fields_index[mid]].hash)
        {
            if(strncasecmp(target, http_header_fields[http_header_fields_index[mid]].str, len) == 0)
                return http_header_fields_index[mid];
            else
                break;
        }
        else if(targ_hash < http_header_fields[http_header_fields_index[mid]].hash)
            high = mid - 1;
        else
            low = mid + 1;
    }
    return -1;
}
