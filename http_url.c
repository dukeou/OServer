#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "logger.h"
#include "list.h"
#include "stack.h"
#include "http_url.h"

#define PRINTABLE_BIT  (1)
#define AL_BIT         (1 << 1)
#define DIGIT_BIT      (1 << 2)
#define HEX_BIT        (1 << 3)
#define UPPER_CASE_BIT (1 << 4)
#define LOWER_CASE_BIT (1 << 5)
#define PATH_BIT       (1 << 6)
#define PATH_BIT       (1 << 6)

#define HTTP_SCHEME "http:"


static char http_ascii[256] = 
{
    /* 0  - 15, 0x00 - 0x0F */
    /*
    NUL, SOH, STX, ETX, EOT, ENQ, ACK, BEL, BS, HT, LF, VT, FF, CR, SO, SI,
    */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 16 - 31, 0x10 - 0x1F */
    /*
    DLE, DC1, DC2, DC3, DC4, NAK, SYN, ETB, CAN, EM, SUB, ESC, FS, GS, RS, US,
    */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 32 - 47, 0x20 - 0x2F */
    /* ' ' */ PRINTABLE_BIT,
    /* '!' */ PRINTABLE_BIT,
    /* '"' */ PRINTABLE_BIT,
    /* '#' */ PRINTABLE_BIT,
    /* '$' */ PRINTABLE_BIT,
    /* '%' */ PRINTABLE_BIT,
    /* '&' */ PRINTABLE_BIT,
    /* ''' */ PRINTABLE_BIT,
    /* '(' */ PRINTABLE_BIT,
    /* ')' */ PRINTABLE_BIT,
    /* '*' */ PRINTABLE_BIT,
    /* '+' */ PRINTABLE_BIT,
    /* ',' */ PRINTABLE_BIT,
    /* '-' */ PRINTABLE_BIT | PATH_BIT,
    /* '.' */ PRINTABLE_BIT | PATH_BIT,
    /* '/' */ PRINTABLE_BIT | PATH_BIT,
    /* 48 - 57, 0x30 - 0x39 */
    /* '0' */ PRINTABLE_BIT | DIGIT_BIT | HEX_BIT | PATH_BIT,
    /* '1' */ PRINTABLE_BIT | DIGIT_BIT | HEX_BIT | PATH_BIT,
    /* '2' */ PRINTABLE_BIT | DIGIT_BIT | HEX_BIT | PATH_BIT,
    /* '3' */ PRINTABLE_BIT | DIGIT_BIT | HEX_BIT | PATH_BIT,
    /* '4' */ PRINTABLE_BIT | DIGIT_BIT | HEX_BIT | PATH_BIT,
    /* '5' */ PRINTABLE_BIT | DIGIT_BIT | HEX_BIT | PATH_BIT,
    /* '6' */ PRINTABLE_BIT | DIGIT_BIT | HEX_BIT | PATH_BIT,
    /* '7' */ PRINTABLE_BIT | DIGIT_BIT | HEX_BIT | PATH_BIT,
    /* '8' */ PRINTABLE_BIT | DIGIT_BIT | HEX_BIT | PATH_BIT,
    /* '9' */ PRINTABLE_BIT | DIGIT_BIT | HEX_BIT | PATH_BIT,
    /* 58 - 64, 0x3A - 0x40 */
    /* ':' */ PRINTABLE_BIT,
    /* ';' */ PRINTABLE_BIT,
    /* '<' */ PRINTABLE_BIT,
    /* '=' */ PRINTABLE_BIT,
    /* '>' */ PRINTABLE_BIT,
    /* '?' */ PRINTABLE_BIT,
    /* '@' */ PRINTABLE_BIT,
    /* 65 - 90, 0x41 - 0x5A */
    /* 'A' */ PRINTABLE_BIT | UPPER_CASE_BIT | AL_BIT | PATH_BIT | HEX_BIT,
    /* 'B' */ PRINTABLE_BIT | UPPER_CASE_BIT | AL_BIT | PATH_BIT | HEX_BIT,
    /* 'C' */ PRINTABLE_BIT | UPPER_CASE_BIT | AL_BIT | PATH_BIT | HEX_BIT,
    /* 'D' */ PRINTABLE_BIT | UPPER_CASE_BIT | AL_BIT | PATH_BIT | HEX_BIT,
    /* 'E' */ PRINTABLE_BIT | UPPER_CASE_BIT | AL_BIT | PATH_BIT | HEX_BIT,
    /* 'F' */ PRINTABLE_BIT | UPPER_CASE_BIT | AL_BIT | PATH_BIT | HEX_BIT,
    /* 'G' */ PRINTABLE_BIT | UPPER_CASE_BIT | AL_BIT | PATH_BIT,
    /* 'H' */ PRINTABLE_BIT | UPPER_CASE_BIT | AL_BIT | PATH_BIT,
    /* 'I' */ PRINTABLE_BIT | UPPER_CASE_BIT | AL_BIT | PATH_BIT,
    /* 'J' */ PRINTABLE_BIT | UPPER_CASE_BIT | AL_BIT | PATH_BIT,
    /* 'K' */ PRINTABLE_BIT | UPPER_CASE_BIT | AL_BIT | PATH_BIT,
    /* 'L' */ PRINTABLE_BIT | UPPER_CASE_BIT | AL_BIT | PATH_BIT,
    /* 'M' */ PRINTABLE_BIT | UPPER_CASE_BIT | AL_BIT | PATH_BIT,
    /* 'N' */ PRINTABLE_BIT | UPPER_CASE_BIT | AL_BIT | PATH_BIT,
    /* 'O' */ PRINTABLE_BIT | UPPER_CASE_BIT | AL_BIT | PATH_BIT,
    /* 'P' */ PRINTABLE_BIT | UPPER_CASE_BIT | AL_BIT | PATH_BIT,
    /* 'Q' */ PRINTABLE_BIT | UPPER_CASE_BIT | AL_BIT | PATH_BIT,
    /* 'R' */ PRINTABLE_BIT | UPPER_CASE_BIT | AL_BIT | PATH_BIT,
    /* 'S' */ PRINTABLE_BIT | UPPER_CASE_BIT | AL_BIT | PATH_BIT,
    /* 'T' */ PRINTABLE_BIT | UPPER_CASE_BIT | AL_BIT | PATH_BIT,
    /* 'U' */ PRINTABLE_BIT | UPPER_CASE_BIT | AL_BIT | PATH_BIT,
    /* 'V' */ PRINTABLE_BIT | UPPER_CASE_BIT | AL_BIT | PATH_BIT,
    /* 'W' */ PRINTABLE_BIT | UPPER_CASE_BIT | AL_BIT | PATH_BIT,
    /* 'X' */ PRINTABLE_BIT | UPPER_CASE_BIT | AL_BIT | PATH_BIT,
    /* 'Y' */ PRINTABLE_BIT | UPPER_CASE_BIT | AL_BIT | PATH_BIT,
    /* 'Z' */ PRINTABLE_BIT | UPPER_CASE_BIT | AL_BIT | PATH_BIT,
    /* 91 - 96, 0x5B - 0x60 */
    /* '[' */ PRINTABLE_BIT,
    /* '\' */ PRINTABLE_BIT,
    /* ']' */ PRINTABLE_BIT,
    /* '^' */ PRINTABLE_BIT,
    /* '_' */ PRINTABLE_BIT | PATH_BIT,
    /* '`' */ PRINTABLE_BIT,
    /* 97 - 122, 0x61 - 0x7A */
    /* 'a' */ PRINTABLE_BIT | LOWER_CASE_BIT | AL_BIT | PATH_BIT | HEX_BIT,
    /* 'b' */ PRINTABLE_BIT | LOWER_CASE_BIT | AL_BIT | PATH_BIT | HEX_BIT,
    /* 'c' */ PRINTABLE_BIT | LOWER_CASE_BIT | AL_BIT | PATH_BIT | HEX_BIT,
    /* 'd' */ PRINTABLE_BIT | LOWER_CASE_BIT | AL_BIT | PATH_BIT | HEX_BIT,
    /* 'e' */ PRINTABLE_BIT | LOWER_CASE_BIT | AL_BIT | PATH_BIT | HEX_BIT,
    /* 'f' */ PRINTABLE_BIT | LOWER_CASE_BIT | AL_BIT | PATH_BIT | HEX_BIT,
    /* 'g' */ PRINTABLE_BIT | LOWER_CASE_BIT | AL_BIT | PATH_BIT,
    /* 'h' */ PRINTABLE_BIT | LOWER_CASE_BIT | AL_BIT | PATH_BIT,
    /* 'i' */ PRINTABLE_BIT | LOWER_CASE_BIT | AL_BIT | PATH_BIT,
    /* 'j' */ PRINTABLE_BIT | LOWER_CASE_BIT | AL_BIT | PATH_BIT,
    /* 'k' */ PRINTABLE_BIT | LOWER_CASE_BIT | AL_BIT | PATH_BIT,
    /* 'l' */ PRINTABLE_BIT | LOWER_CASE_BIT | AL_BIT | PATH_BIT,
    /* 'm' */ PRINTABLE_BIT | LOWER_CASE_BIT | AL_BIT | PATH_BIT,
    /* 'n' */ PRINTABLE_BIT | LOWER_CASE_BIT | AL_BIT | PATH_BIT,
    /* 'o' */ PRINTABLE_BIT | LOWER_CASE_BIT | AL_BIT | PATH_BIT,
    /* 'p' */ PRINTABLE_BIT | LOWER_CASE_BIT | AL_BIT | PATH_BIT,
    /* 'q' */ PRINTABLE_BIT | LOWER_CASE_BIT | AL_BIT | PATH_BIT,
    /* 'r' */ PRINTABLE_BIT | LOWER_CASE_BIT | AL_BIT | PATH_BIT,
    /* 's' */ PRINTABLE_BIT | LOWER_CASE_BIT | AL_BIT | PATH_BIT,
    /* 't' */ PRINTABLE_BIT | LOWER_CASE_BIT | AL_BIT | PATH_BIT,
    /* 'u' */ PRINTABLE_BIT | LOWER_CASE_BIT | AL_BIT | PATH_BIT,
    /* 'v' */ PRINTABLE_BIT | LOWER_CASE_BIT | AL_BIT | PATH_BIT,
    /* 'w' */ PRINTABLE_BIT | LOWER_CASE_BIT | AL_BIT | PATH_BIT,
    /* 'x' */ PRINTABLE_BIT | LOWER_CASE_BIT | AL_BIT | PATH_BIT,
    /* 'y' */ PRINTABLE_BIT | LOWER_CASE_BIT | AL_BIT | PATH_BIT,
    /* 'z' */ PRINTABLE_BIT | LOWER_CASE_BIT | AL_BIT | PATH_BIT,
    /* 123 - 126, 0x7B - 0x7E */
    /* '{' */ PRINTABLE_BIT,
    /* '|' */ PRINTABLE_BIT,
    /* '}' */ PRINTABLE_BIT,
    /* '~' */ PRINTABLE_BIT,
    /* 127, 0x7F */
    /* DEL */ 0,
    /* 128 - 255, 0x80 - 0xFF */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};
#define IS_PCHAR(x)  (http_ascii[(unsigned char)(x)] & PRINTABLE_BIT)
#define IS_NUM(x)    (http_ascii[(unsigned char)(x)] & DIGIT_BIT)
#define IS_LETTER(x) (http_ascii[(unsigned char)(x)] & AL_BIT)
#define IS_ALNUM(x)  (IS_NUM(x) || IS_LETTER(x))
#define IS_PATH(x)   (http_ascii[(unsigned char)(x)] & PATH_BIT)
static char http_hex[] = "0123456789ABCDEF";

static int http_url_parse_path(char *data, int size, int *parse_len, http_url_path_info_t *path);
static int http_url_parse_query(char *data, int size, int *parse_len, http_url_query_info_t *query);

char http_hex_char_to_num(unsigned char x)
{
    char bits = http_ascii[(unsigned int)x];
    if(bits & DIGIT_BIT)
        x -= '0';
    else if(bits & HEX_BIT)
        if(bits & LOWER_CASE_BIT)
            x -= 'a' - 10;
        else
            x -= 'A' - 10;
    else
        x = -1;
    return (char)x;
}
int http_url_unescape(char *url)
{
    char *url_decoded = url, *p = url, x, y;
    for(; *p; ++p)
    {
        if(*p == '%')
        {
            x = http_hex_char_to_num(*(++p));
            if(x < 0)
                return -1;
            x <<= 4;
            y = http_hex_char_to_num(*(++p));
            if(y < 0)
                return -1;
            x += y;
            *url_decoded++ = x;
        }
        else
        {
            *url_decoded++ = *p;
        }
    }
    *url_decoded = 0;
    return 0;
}
char *http_url_escape(char *url)
{
    char *url_encoded = NULL, *p = url, *x;
    int len = 0;
    for(; *p; ++p, ++len)
        if(!IS_ALNUM(*p))
            len += 2;
    len += 1;
    url_encoded = (char*)malloc(len);
    if(NULL == url_encoded)
        return NULL;
    for(p = url, x = url_encoded; *p; ++p)
    {
        if(!IS_ALNUM(*p))
        {
            *x++ = '%';
            *x++ = http_hex[(*p) >> 4];
            *x++ = http_hex[(*p) & 0x0F];
        }
        else
        {
            *x++ = *p;
        }
    }
    *x = 0;
    return url_encoded;
}
void http_url_init(http_url_t *url)
{
    memset(url, 0, sizeof(*url));
    stack_init(&url->path.path_stack);
    list_init(&url->query.query_list);
}
void http_url_destory(http_url_t *url)
{
    while(!stack_empty(&url->path.path_stack))
    {
        free(stack_pop(&url->path.path_stack));
    }
    while(!list_empty(&url->query.query_list))
    {
        free(list_remove_first(&url->query.query_list));
    }
    http_url_init(url);
}
int http_url_parse(const char *data, int size, int *parse_len, http_url_t *url)
{
    const char *space = NULL;
    char *urlp, *p;
    int url_len, i, read_len;
    *parse_len = 0;
    for(i = 0; i < size; ++i)
    {
        if(data[i] == 0)
        {
            return -1;
        }
        if(data[i] == ' ')
        {
            space = &data[i];
            break;
        }
    }
    url_len = i;
    if(!space || url_len == 0)
        return 0;
    if(url_len >= sizeof(url->url))
        return -1;
    *parse_len = url_len + 1;
    memcpy(url->url, data, url_len);
    url->url[url_len] = 0;
    urlp = url->url;
    if(strncasecmp(urlp, HTTP_SCHEME, sizeof(HTTP_SCHEME)) == 0)
    {
        urlp += sizeof(HTTP_SCHEME);
    }
    if((p = strstr(urlp, "//")))
    {
        urlp = p + 2;
        if((p = strchr(urlp, '/')))
        {
            url->host = urlp;
            url->host_len = p - urlp;
            urlp = p + 1;
        }
        else
        {
            url->host = urlp;
            url->host_len = url_len - (urlp - url->url);
            return 0;
        }
    }
    else
    {
        if(*urlp == '/')
        {
            urlp += 1;
        }
        else if(*urlp == 0)
        {
            return 0;
        }
        else
        {
            return -1;
        }
    }
    l_og(e_log_level_debug, "url: %s, size: %d\n", urlp, url_len - (urlp - url->url));
    if(0 != http_url_parse_path(urlp, url_len - (urlp - url->url), &read_len, &url->path))
    {
        return -1;
    }
    urlp += read_len;
    l_og(e_log_level_debug, "url: %s, size: %d\n", urlp + 1, url_len - (urlp - url->url));
    if(urlp[0] == '?')
    {
        if(0 != http_url_parse_query(urlp + 1, url_len - (urlp - url->url) - 1, &read_len, &url->query))
        {
            return -1;
        }
    }
    return 0;
}
static int http_url_parse_path(char *data, int size, int *parse_len, http_url_path_info_t *path)
{
    char *p = data, *seg;
    int i, seg_len;
    http_url_path_seg_t *s;
    path->data = p;
    *parse_len = 0;
    i = 0; seg = p;
    if(!size) return 0;
    do
    {
        if(i == size || p[i] == '?' || p[i] == '#' || p[i] == '/')
        {
            seg_len = i - (seg - p) + ((i != size && p[i] == '/')? 1 : 0);
            l_og(e_log_level_debug, "path seg: %s, seg len: %d\n", seg, seg_len);
            if(seg_len && strncmp(seg, "../", seg_len) == 0)
            {
                if(!(s = (http_url_path_seg_t*)stack_pop(&path->path_stack)))
                {
                    l_og(e_log_level_fatal, "Error ../\n");
                    return -1;
                }
                free(s);
            }
            else if(strncmp(seg, "./", seg_len) != 0)
            {
                s = (http_url_path_seg_t*)malloc(sizeof(http_url_path_seg_t));
                s->data = seg;
                s->data_len = seg_len;
                stack_push(&path->path_stack, (stack_node_t*)s);
            }
            if(i == size || p[i] != '/')
            {
                break;
            }
            seg += seg_len;
        }
        else if(!IS_PATH(p[i]))
        {
            l_og(e_log_level_fatal, "Error Non-Path\n");
            return -1;
        }
        ++i;
    }while(1);
    path->data_len = i;
    *parse_len = i;
    return 0;
}
static int http_url_parse_query(char *data, int size, int *parse_len, http_url_query_info_t *query)
{
    char *p, *ns, *ne, *vs, *ve;
    int i;
    http_url_query_item_t *q;
    query->data = data;
    *parse_len = 0;
    ns = ne = vs = ve = NULL;
    i = 0; p = data;
    do
    {
        if(i == size || p[i] == '#' || p[i] == '&')
        {
            if(!vs)
            {
                break;
            }
            ve = &p[i];
            l_og(e_log_level_debug, "name: %s, name len: %d\n", ns, ne - ns);
            l_og(e_log_level_debug, "value: %s, value len: %d\n", vs, ve - vs);
            q = (http_url_query_item_t*)malloc(sizeof(http_url_query_item_t));
            q->name = ns;
            q->name_len = ne - ns;
            q->value = vs;
            q->value_len = ve - vs;
            list_append(&query->query_list, (list_node_t*)q);
            ns = ne = vs = ve = NULL;
            if(i == size)
            {
                break;
            }
        }
        else if(p[i] == '=')
        {
            if(ns)
            {
                ne = &p[i];
            }
        }
        else if(!ns)
        {
            ns = &p[i];
        }
        else if(ne)
        {
            vs = &p[i];
        }
        ++i;
    }while(1);
    query->data_len = i;
    return 0;
}
