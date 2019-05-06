#ifndef O_HTTP_HEADER
#define O_HTTP_HEADER
typedef enum
{
    e_header_accept,
    e_header_accept_charset,
    e_header_accept_encoding,
    e_header_accept_language,
    e_header_accept_ranges,
    e_header_access_control_allow_credentials,
    e_header_access_control_allow_headers,
    e_header_access_control_allow_methods,
    e_header_access_control_allow_origin,
    e_header_access_control_expose_headers,
    e_header_access_control_max_age,
    e_header_access_control_request_headers,
    e_header_access_control_request_method,
    e_header_age,
    e_header_allow,
    e_header_alt_svc,
    e_header_authorization,
    e_header_cache_control,
    e_header_clear_site_data,
    e_header_connection,
    e_header_content_disposition,
    e_header_content_encoding,
    e_header_content_language,
    e_header_content_length,
    e_header_content_location,
    e_header_content_range,
    e_header_content_security_policy,
    e_header_content_security_policy_report_only,
    e_header_content_type,
    e_header_cookie,
    e_header_cookie2,
    e_header_dnt,
    e_header_date,
    e_header_etag,
    e_header_early_data,
    e_header_expect,
    e_header_expect_ct,
    e_header_expires,
    e_header_feature_policy,
    e_header_forwarded,
    e_header_from,
    e_header_host,
    e_header_if_match,
    e_header_if_modified_since,
    e_header_if_none_match,
    e_header_if_range,
    e_header_if_unmodified_since,
    e_header_index,
    e_header_keep_alive,
    e_header_large_allocation,
    e_header_last_modified,
    e_header_location,
    e_header_origin,
    e_header_pragma,
    e_header_proxy_authenticate,
    e_header_proxy_authorization,
    e_header_public_key_pins,
    e_header_public_key_pins_report_only,
    e_header_range,
    e_header_referer,
    e_header_referrer_policy,
    e_header_retry_after,
    e_header_sec_websocket_accept,
    e_header_server,
    e_header_server_timing,
    e_header_set_cookie,
    e_header_set_cookie2,
    e_header_sourcemap,
    e_header_strict_transport_security,
    e_header_te,
    e_header_timing_allow_origin,
    e_header_tk,
    e_header_trailer,
    e_header_transfer_encoding,
    e_header_upgrade_insecure_requests,
    e_header_user_agent,
    e_header_vary,
    e_header_via,
    e_header_www_authenticate,
    e_header_warning,
    e_header_x_content_type_options,
    e_header_x_dns_prefetch_control,
    e_header_x_forwarded_for,
    e_header_x_forwarded_host,
    e_header_x_forwarded_proto,
    e_header_x_frame_options,
    e_header_x_xss_protection,
}http_header_field_id_t;
typedef struct
{
    const char *str;
    int16_t len;
    int16_t hash;
}http_header_field_t;
typedef struct http_header
{
    union
    {
        bt_key_t        key;
        bt_node_entry_t padding;
    };
    char *value;
    int16_t len;
}http_header_t;
typedef struct
{
    btree_t       header_tree;
    http_header_t *traverse_ptr;
}http_header_collection_t;
typedef struct
{
    char *buf;
    int size;
    http_header_collection_t *headers;
}http_header_str_info_t;
void http_header_init(http_header_collection_t *headers);
int http_header_destroy(http_header_collection_t *headers);
int http_header_add(http_header_collection_t *headers, const char *name, int ln, const char *value, int lv);
int http_header_add_by_id(http_header_collection_t *headers, int id, const char *value, int lv);
int http_header_len(http_header_collection_t *headers);
int http_header_to_str(http_header_collection_t *headers, char *s, int size, int *is_end);
int http_header_parse(http_header_collection_t *headers, const char *buf, int len, int *rdlen, int *is_end);
void http_header_hash_all_fields();
void http_header_fields_bubble_sort();
int16_t http_header_hash(const char *str);
int16_t http_header_n_hash(const char *bin, int len);
int http_header_binary_search(const char *target);
int http_header_binary_n_search(const char *target, int len);
#endif /* O_HTTP_HEADER */
