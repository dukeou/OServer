#ifndef O_SERVER
#define O_SERVER
#include "server_config.h"
typedef struct
{
    server_config_t config;
    thread_pool_t   threads;
    object_pool_t   connections;
    list_t          conn_list;
    pthread_mutex_t fd_lock;
    fd_set          fds;
    int             listen_fd;
    int             read_fd;
    int             write_fd;
    struct sockaddr addr;
}server_t;
typedef struct connection_info
{
    struct connection_info *prev;
    struct connection_info *next;
    server_t               *srv;
    int                     socket;
    struct sockaddr_storage peer_addr;
    http_request_t          request;
    http_response_t         response;
    int                     data_len;
    char                    data_buffer[1024];
}connection_info_t;
void server_entry(server_t* srv);
void server_clean(server_t *srv);
int server_listen(server_t *srv);
int server_open_pipe(server_t *srv);
void server_notify(server_t *srv);
int server_check_conn_in(connection_info_t *conn, fd_set *rfds);
int server_close_conn(connection_info_t *conn, void *arg);
void server_serv_client(void *data);
void server_process_request(server_t *srv, http_request_t *req, http_response_t *res, int *keep_alive);
void server_process_get(server_t *srv, http_request_t *req, http_response_t *res, int *keep_alive);
void server_process_head(server_t *srv, http_request_t *req, http_response_t *res, int *keep_alive);
void server_process_options(server_t *srv, http_request_t *req, http_response_t *res, int *keep_alive);
#endif /* O_SERVER */
