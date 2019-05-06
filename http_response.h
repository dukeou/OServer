#ifndef O_HTTP_RESPONSE
#define O_HTTP_RESPONSE
typedef enum
{
    e_http_100_continue = 0,
    e_http_101_switching_protocols,
    e_http_200_ok,
    e_http_201_created,
    e_http_202_accepted,
    e_http_203_non_authoritative_information,
    e_http_204_no_content,
    e_http_205_reset_content,
    e_http_206_partial_content,
    e_http_300_multiple_choices,
    e_http_301_moved_permanently,
    e_http_302_found,
    e_http_303_see_other,
    e_http_304_not_modified,
    e_http_305_use_proxy,
    e_http_307_temporary_redirect,
    e_http_400_bad_request,
    e_http_401_unauthorized,
    e_http_402_payment_required,
    e_http_403_forbidden,
    e_http_404_not_found,
    e_http_405_method_not_allowed,
    e_http_406_not_acceptable,
    e_http_407_proxy_authentication_required,
    e_http_408_request_timeout,
    e_http_409_conflict,
    e_http_410_gone,
    e_http_411_length_required,
    e_http_412_precondition_failed,
    e_http_413_request_entity_too_large,
    e_http_414_request_uri_too_long,
    e_http_415_unsupported_media_type,
    e_http_416_requested_range_not_satisfiable,
    e_http_417_expectation_failed,
    e_http_500_internal_server_error,
    e_http_501_not_implemented,
    e_http_502_bad_gateway,
    e_http_503_service_unavailable,
    e_http_504_gateway_timeout,
    e_http_505_http_version_not_supported,
}http_status_t;
typedef enum
{
    e_http_response_gen_status_begin,
    e_http_response_gen_status_version = e_http_response_gen_status_begin,
    e_http_response_gen_status_code,
    e_http_response_gen_status_headers,
    e_http_response_gen_status_content,
    e_http_response_gen_status_error,
}http_response_gen_status_t;
typedef struct
{
    http_version_t             version;
    http_status_t              status;
    http_header_collection_t   headers;
    http_response_gen_status_t gen_status;
    int                        content;
}http_response_t;
typedef enum
{
    e_trasfer_encoding_none,
    e_trasfer_encoding_trunk = -1,
}http_trasfer_encoding_t;
int http_response_init(http_response_t *res, http_status_t code);
int http_response_reset(http_response_t *res, http_status_t code);
int http_response_gen(http_response_t *res, char *buf, int size, int *is_end);
void http_response_log(http_response_t *res);
#endif /* O_HTTP_RESPONSE */
