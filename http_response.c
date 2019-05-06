#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "btree.h"
#include "logger.h"
#include "http_version.h"
#include "http_header.h"
#include "http_response.h"

static const char *http_response_code[] = 
{
    /*
     * the client s_hould continue with its request. this interim response is
     * used to inform the client that the initial part of the request has
     * been received and has not yet been rejected by the server. the client
     * s_hould continue by sending the remainder of the request or, if the
     * request has already been completed, ignore this response. the server
     * m_ust send a final response after the request has been completed. see
     * section 8.2.3 for detailed discussion of the use and handling of this
     * status code.
     */
    "100 Continue",
    /*
     * the server understands and is willing to comply with the client's
     * request, via the upgrade message header field (section 14.42), for a
     * change in the application protocol being used on this connection. the
     * server will switch protocols to those defined by the response's
     * upgrade header field immediately after the empty line which
     * terminates the 101 response.
     *
     * the protocol s_hould be switched only when it is advantageous to do
     * so. for example, switching to a newer version of h_ttp is advantageous
     * over older versions, and switching to a real-time, synchronous
     * protocol might be advantageous when delivering resources that use
     * such features.
     */
    "101 Switching Protocols",
    /*
     * the request has succeeded. the information returned with the response
     * is dependent on the method used in the request, for example:
     *
     * g_et    an entity corresponding to the requested resource is sent in
     * the response;
     *
     * h_ead   the entity-header fields corresponding to the requested
     * resource are sent in the response without any message-body;
     *
     * p_ost   an entity describing or containing the result of the action;
     *
     * t_race  an entity containing the request message as received by the
     *           end server.
     */
    "200 OK",
    /*
     * the request has been fulfilled and resulted in a new resource being
     * created. the newly created resource can be referenced by the u_ri(s)
     * returned in the entity of the response, with the most specific u_ri
     * for the resource given by a location header field. the response
     * s_hould include an entity containing a list of resource
     * characteristics and location(s) from which the user or user agent can
     * choose the one most appropriate. the entity format is specified by
     * the media type given in the content-type header field. the origin
     * server m_ust create the resource before returning the 201 status code.
     * if the action cannot be carried out immediately, the server s_hould
     * respond with 202 (accepted) response instead.
     *
     * a 201 response m_ay contain an e_tag response header field indicating
     * the current value of the entity tag for the requested variant just
     * created, see section 14.19.
     */
    "201 Created",
    /*
     * the request has been accepted for processing, but the processing has
     * not been completed.  the request might or might not eventually be
     * acted upon, as it might be disallowed when processing actually takes
     * place. there is no facility for re-sending a status code from an
     * asynchronous operation such as this.
     *
     * the 202 response is intentionally non-committal. its purpose is to
     * allow a server to accept a request for some other process (perhaps a
     * batch-oriented process that is only run once per day) without
     * requiring that the user agent's connection to the server persist
     * until the process is completed. the entity returned with this
     * response s_hould include an indication of the request's current status
     * and either a pointer to a status monitor or some estimate of when the
     * user can expect the request to be fulfilled.
     */
    "202 Accepted",
    /*
     * the returned metainformation in the entity-header is not the
     * definitive set as available from the origin server, but is gathered
     * from a local or a third-party copy. the set presented m_ay be a subset
     * or superset of the original version. for example, including local
     * annotation information about the resource might result in a superset
     * of the metainformation known by the origin server. use of this
     * response code is not required and is only appropriate when the
     * response would otherwise be 200 (ok).
     */
    "203 Non-authoritative Information",
    /*
     * the server has fulfilled the request but does not need to return an
     * entity-body, and might want to return updated metainformation. the
     * response m_ay include new or updated metainformation in the form of
     * entity-headers, which if present s_hould be associated with the
     * requested variant.
     *
     * if the client is a user agent, it s_hould n_ot change its document view
     * from that which caused the request to be sent. this response is
     * primarily intended to allow input for actions to take place without
     * causing a change to the user agent's active document view, although
     * any new or updated metainformation s_hould be applied to the document
     * currently in the user agent's active view.
     *
     * the 204 response m_ust n_ot include a message-body, and thus is always
     * terminated by the first empty line after the header fields.
     */
    "204 No Content",
    /*
     * the server has fulfilled the request and the user agent s_hould reset
     * the document view which caused the request to be sent. this response
     * is primarily intended to allow input for actions to take place via
     * user input, followed by a clearing of the form in which the input is
     * given so that the user can easily initiate another input action. the
     * response m_ust n_ot include an entity.
     */
    "205 Reset Content",
    /*
     */
    "206 Partial Content",
    /*
     */
    "300 Multiple Choices",
    /*
     * the requested resource has been assigned a new permanent u_ri and any
     * future references to this resource s_hould use one of the returned
     * u_ris.  clients with link editing capabilities ought to automatically
     * re-link references to the request-uri to one or more of the new
     * references returned by the server, where possible. this response is
     * cacheable unless indicated otherwise.
     *
     * the new permanent u_ri s_hould be given by the location field in the
     * response. unless the request method was h_ead, the entity of the
     * response s_hould contain a short hypertext note with a hyperlink to
     * the new u_ri(s).
     *
     * if the 301 status code is received in response to a request other
     * than g_et or h_ead, the user agent m_ust n_ot automatically redirect the
     * request unless it can be confirmed by the user, since this might
     * change the conditions under which the request was issued.
     *
     * note: when automatically redirecting a p_ost request after
     * receiving a 301 status code, some existing h_ttp/1.0 user agents
     * will erroneously change it into a g_et request.
     */
    "301 Moved Permanently",
    /*
     */
    "302 Found",
    /*
     */
    "303 See Other",
    /*
     */
    "304 Not Modified",
    /*
     */
    "305 Use Proxy",
    /*
     */
    "307 Temporary Redirect",
    /*
     * the request could not be understood by the server due to malformed
     * syntax. the client s_hould n_ot repeat the request without
     * modifications.
     */
    "400 Bad Request",
    /*
     * the request requires user authentication. the response m_ust include a
     * w_ww-authenticate header field (section 14.47) containing a challenge
     * applicable to the requested resource. the client m_ay repeat the
     * request with a suitable authorization header field (section 14.8). if
     * the request already included authorization credentials, then the 401
     * response indicates that authorization has been refused for those
     * credentials. if the 401 response contains the same challenge as the
     * prior response, and the user agent has already attempted
     * authentication at least once, then the user s_hould be presented the
     * entity that was given in the response, since that entity might
     * include relevant diagnostic information. h_ttp access authentication
     * is explained in "http authentication: basic and digest access
     */
    "401 Unauthorized",
    /*
     */
    "402 Payment Required",
    /*
     * the server understood the request, but is refusing to fulfill it.
     * authorization will not help and the request s_hould n_ot be repeated.
     * if the request method was not h_ead and the server wishes to make
     * public why the request has not been fulfilled, it s_hould describe the
     * reason for the refusal in the entity.  if the server does not wish to
     * make this information available to the client, the status code 404
     * (not found) can be used instead.
     */
    "403 Forbidden",
    /*
     * the server has not found anything matching the request-uri. no
     * indication is given of whether the condition is temporary or
     * permanent. the 410 (gone) status code s_hould be used if the server
     * knows, through some internally configurable mechanism, that an old
     * resource is permanently unavailable and has no forwarding address.
     * this status code is commonly used when the server does not wish to
     * reveal exactly why the request has been refused, or when no other
     * response is applicable.
     */
    "404 Not Found",
    /*
     * the method specified in the request-line is not allowed for the
     * resource identified by the request-uri. the response m_ust include an
     * allow header containing a list of valid methods for the requested
     * resource.
     */
    "405 Method Not Allowed",
    /*
     */
    "406 Not Acceptable",
    /*
     */
    "407 Proxy Authentication Required",
    /*
     */
    "408 Request Timeout",
    /*
     */
    "409 Conflict",
    /*
     */
    "410 Gone",
    /*
     */
    "411 Length Required",
    /*
     * the server refuses to accept the request without a defined content-
     * length. the client m_ay repeat the request if it adds a valid
     * content-length header field containing the length of the message-body
     * in the request message.
     */
    "412 Precondition Failed",
    /*
     * the server is refusing to process a request because the request
     * entity is larger than the server is willing or able to process. the
     * server m_ay close the connection to prevent the client from continuing
     * the request.
     *
     * if the condition is temporary, the server s_hould include a retry-
     * after header field to indicate that it is temporary and after what
     * time the client m_ay try again.
     */
    "413 Request Entity Too Large",
    /*
     * the server is refusing to service the request because the request-uri
     * is longer than the server is willing to interpret. this rare
     * condition is only likely to occur when a client has improperly
     * converted a p_ost request to a g_et request with long query
     * information, when the client has descended into a u_ri "black hole" of
     * redirection (e.g., a redirected u_ri prefix that points to a suffix of
     * itself), or when the server is under attack by a client attempting to
     * exploit security holes present in some servers using fixed-length
     * buffers for reading or manipulating the request-uri.
     */
    "414 Request-uri Too Long",
    /*
     */
    "415 Unsupported Media Type",
    /*
     */
    "416 Requested Range Not Satisfiable",
    /*
     */
    "417 Expectation Failed",
    /*
     * the server encountered an unexpected condition which prevented it
     * from fulfilling the request.
     */
    "500 Internal Server Error",
    /*
     */
    "501 Not Implemented",
    /*
     */
    "502 Bad Gateway",
    /*
     * the server is currently unable to handle the request due to a
     * temporary overloading or maintenance of the server. the implication
     * is that this is a temporary condition which will be alleviated after
     * some delay. if known, the length of the delay m_ay be indicated in a
     * retry-after header. if no retry-after is given, the client s_hould
     * handle the response as it would for a 500 response.
     *
     * note: the existence of the 503 status code does not imply that a
     * server must use it when becoming overloaded. some servers may wish
     * to simply refuse the connection.
     */
    "503 Service Unavailable",
    /*
     */
    "504 Gateway Timeout",
    /*
     * the server does not support, or refuses to support, the h_ttp protocol
     * version that was used in the request message. the server is
     * indicating that it is unable or unwilling to complete the request
     * using the same major version as the client, as described in section
     * 3.1, other than with this error message. the response s_hould contain
     * an entity describing why that version is not supported and what other
     * protocols are supported by that server.
     */
    "505 Http Version Not Supported",
};
int http_response_init(http_response_t *res, http_status_t code)
{
    res->version = e_http_version_1_1;
    res->status = code;
    res->content = -1;
    res->gen_status = e_http_response_gen_status_begin;
    http_header_init(&res->headers);
    return 0;
}
int http_response_reset(http_response_t *res, http_status_t code)
{
    res->version = e_http_version_1_1;
    res->status = code;
    res->gen_status = e_http_response_gen_status_version;
    if(res->content >= 0)
    {
        close(res->content);
        res->content = -1;
    }
    http_header_destroy(&res->headers);
    return 0;
}
int http_response_gen(http_response_t *res, char *buf, int size, int *is_end)
{
    int len, is_header_end;
    char *p;
    http_response_gen_status_t status;
    is_header_end = 0;
    p = buf;
    *is_end = 0;
    do
    {
        status = res->gen_status;
        switch(res->gen_status)
        {
            case e_http_response_gen_status_version:
                if(size >= http_version_len(res->version))
                {
                    len = http_version_to_str(res->version, p, size);
                    size -= len;
                    p += len;
                    res->gen_status = e_http_response_gen_status_code;
                }
                break;
            case e_http_response_gen_status_code:
                len = strlen(http_response_code[res->status]);
                if(size >= len + 2) 
                {
                    strncpy(p, http_response_code[res->status], size);
                    size -= len;
                    p += len;
                    *p++ = '\r';
                    *p++ = '\n';
                    size -= 2;
                    res->gen_status = e_http_response_gen_status_headers;
                }
                break;
            case e_http_response_gen_status_headers:
                len = http_header_to_str(&res->headers, p, size, &is_header_end);
                size -= len;
                p += len;
                if(is_header_end)
                {
                    res->gen_status = e_http_response_gen_status_content;
                }
                break;
            case e_http_response_gen_status_content:
                if(res->content > 0)
                {
                    len = read(res->content, p, size);
                    if(len <= 0)
                    {
                        *is_end = 1;
                    }
                    else
                    {
                        size -= len;
                        p += len;
                    }
                }
                else
                {
                    *is_end = 1;
                }
                break;
            default:
                res->gen_status = e_http_response_gen_status_error;
                *is_end = 1;
                break;
        }
    }while(status != res->gen_status && res->gen_status != e_http_response_gen_status_error);
    return p - buf;
}
void http_response_log(http_response_t *res)
{
    int len, size, is_end;
    char *buf, *hptr;
    size = http_header_len(&res->headers);
    size += http_version_len(res->version);
    size += 2;
    buf = (char*)malloc(size);
    len = http_version_to_str(res->version, buf, size);
    buf[len] = '\0';
    hptr = buf + len + 1;
    len = http_header_to_str(&res->headers, hptr, buf + size - hptr, &is_end);
    hptr[len] = '\0';
    l_og(e_log_level_info, "\n%s%s\n%s", buf, http_response_code[res->status], hptr);
    l_og(e_log_level_debug, "http_response_log end\n");
    free(buf);
}
