#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>

#include "btree.h"
#include "list.h"
#include "stack.h"
#include "server_config.h"
#include "logger.h"
#include "http_header.h"
#include "http_version.h"
#include "http_url.h"
#include "http_method.h"
#include "sync_queue.h"
#include "thread_pool.h"
#include "http_request.h"
#include "http_response.h"
#include "object_pool.h"
#include "list.h"
#include "server.h"

void server_entry(server_t* srv)
{
    int max_fd, client_fd, ret;
    char one_byte;
    fd_set rfds;
    socklen_t addr_len;
    connection_info_t *conn;
    struct sockaddr_storage peer_addr;
    /*
     * server init
     */
    if(server_listen(srv) != 0)
        return;
    if(server_open_pipe(srv) != 0)
        return;
    object_pool_create(&srv->connections, sizeof(connection_info_t), srv->config.num_max_connections);
    thread_pool_init(&srv->threads, srv->config.num_max_threads, srv->config.num_regular_threads);
    pthread_mutex_init(&srv->fd_lock, NULL);
    list_init(&srv->conn_list);
    FD_ZERO(&srv->fds);
    FD_SET(srv->listen_fd, &srv->fds);
    FD_SET(srv->read_fd, &srv->fds);

    addr_len = sizeof(struct sockaddr_storage);
    max_fd = srv->listen_fd;
    if(srv->read_fd > max_fd)
        max_fd = srv->read_fd;
    for(;;)
    {
        pthread_mutex_lock(&srv->fd_lock);
        memcpy(&rfds, &srv->fds, sizeof(fd_set));
        pthread_mutex_unlock(&srv->fd_lock);
        l_og(e_log_level_debug, "select enter\n");
        ret = select(max_fd + 1, &rfds, NULL, NULL, NULL);
        l_og(e_log_level_debug, "select out with ret %d\n", ret);
        if(ret < 0)
        {
            l_og_error("select");
            break;
        }
        if(ret == 0)
            continue;
        if(FD_ISSET(srv->read_fd, &rfds))
        {
            if(1 != read(srv->read_fd, &one_byte, 1))
            {
                l_og_error("read");
            }
            if(ret == 1)
                continue;
        }
        if(FD_ISSET(srv->listen_fd, &rfds))
        {
            client_fd = accept(srv->listen_fd, (struct sockaddr*)&peer_addr, &addr_len);
            if(client_fd < 0)
            {
                l_og_error("accept");
                continue;
            }
            l_og(e_log_level_info, "client: %s, port: %d, socket: %d\n",
                    inet_ntoa(((struct sockaddr_in*)&peer_addr)->sin_addr), 
                    ntohs(((struct sockaddr_in*)&peer_addr)->sin_port), client_fd);
            if(client_fd > max_fd)
                max_fd = client_fd;
            FD_SET(client_fd, &srv->fds);
            conn = (connection_info_t*)object_pool_acquire(&srv->connections);
            if(NULL == conn)
            {
                l_og(e_log_level_info, "connection pool full\n");
            }
            else
            {
                l_og(e_log_level_debug, "Create new connection %x\n", conn);
                conn->srv = srv;
                conn->socket = client_fd;
                conn->data_len = 0;
                memcpy(&conn->peer_addr, &peer_addr, sizeof(conn->peer_addr));
                http_request_init(&conn->request);
                http_response_init(&conn->response, e_http_200_ok);
                list_append(&srv->conn_list, (list_node_t*)conn);
                l_og(e_log_level_debug, "Create new connection end\n");
            }
        }
        list_traverse(&srv->conn_list, (list_visit_func)server_check_conn_in, &rfds);
    }
    server_clean(srv);
    l_og(e_log_level_debug, "server end\n");
}
void server_clean(server_t *srv)
{
    list_traverse(&srv->conn_list, (list_visit_func)server_close_conn, NULL);
    thread_pool_destroy(&srv->threads);
    object_pool_destroy(&srv->connections);
    pthread_mutex_destroy(&srv->fd_lock);
    close(srv->listen_fd);
    close(srv->read_fd);
    close(srv->write_fd);
}
int server_listen(server_t *srv)
{
    int ret, opt_value;
    char node[256];
    char service[10];
    struct addrinfo hints;
    struct addrinfo *result, *ptr;

    ret = -1;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags    = AI_PASSIVE;
    hints.ai_protocol = 0;
    if(0 != gethostname(node, sizeof(node)))
    {
        l_og_error("gethostname");
        return -1;
    }
    l_og(e_log_level_info, "hostname: %s\n", node);
    sprintf(service, "%d", srv->config.port);
    if(0 != getaddrinfo(node, service, &hints, &result))
    {
        l_og_error("getaddrinfo");
        return -1;
    }
    for(ptr = result; ptr != NULL; ptr = ptr->ai_next)
    {
        srv->listen_fd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if(-1 == srv->listen_fd)
        {
            l_og_error("socket");
            continue;
        }
        opt_value = 1;
        if (setsockopt(srv->listen_fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt_value, sizeof(opt_value))) {
            l_og_error("setsockopt");
        }
        if(0 == bind(srv->listen_fd, ptr->ai_addr, ptr->ai_addrlen))
        {
            l_og(e_log_level_info, "address: %s, port: %d, root: %s\n",
                    inet_ntoa(((struct sockaddr_in*)ptr->ai_addr)->sin_addr),
                    srv->config.port,
                    srv->config.root);
            break;
        }
        l_og_error("bind");
        close(srv->listen_fd);
    }
    if(NULL == ptr)
    {
        l_og(e_log_level_fatal, "No available node\n");
    }
    else if(0 != listen(srv->listen_fd, srv->config.backlog))
    {
        l_og_error("listen");
        close(srv->listen_fd);
        srv->listen_fd = -1;
    }
    else
    {
        memcpy(&srv->addr, ptr->ai_addr, sizeof(srv->addr));
        ret = 0;
    }
    freeaddrinfo(result);
    return ret;
}
int server_open_pipe(server_t *srv)
{
    int pipefd[2];
    if(0 != pipe(pipefd))
    {
        l_og_error("pipe");
        return -1;
    }
    srv->read_fd = pipefd[0];
    srv->write_fd = pipefd[1];
    return 0;
}
void server_notify(server_t *srv)
{
    char one_byte = 'X';
    if(1 != write(srv->write_fd, &one_byte, 1))
    {
        l_og_error("write");
    }
}
int server_check_conn_in(connection_info_t *conn, fd_set *rfds)
{
    l_og(e_log_level_debug, "server_check_conn_in conn: 0x%X\n", conn);
    l_og(e_log_level_debug, "server_check_conn_in fd: %d\n", conn->socket);
    if(-1 == conn->socket)
    {
        http_request_reset(&conn->request);
        http_response_reset(&conn->response, e_http_200_ok);
        list_remove(&conn->srv->conn_list, (list_node_t*)conn);
        object_pool_return(&conn->srv->connections, (object_t*)conn);
    }
    else if(FD_ISSET(conn->socket, rfds))
    {
        l_og(e_log_level_debug, "server_check_conn_in fd: %d is set\n", conn->socket);
        pthread_mutex_lock(&conn->srv->fd_lock);
        FD_CLR(conn->socket, &conn->srv->fds);
        pthread_mutex_unlock(&conn->srv->fd_lock);
        thread_pool_add_task(&conn->srv->threads, server_serv_client, conn);
    }
    return 1;
}
int server_close_conn(connection_info_t *conn, void *arg)
{
    close(conn->socket);
    conn->socket = -1;
    return 1;
}
void server_serv_client(void *data)
{
    int ret, parse_end, send_end, rdlen, remain, rbuf_size, wbuf_size, wp, keep_alive;
    char rbuf[2048], wbuf[4096];
    connection_info_t *conn;
    conn       = (connection_info_t*)data;
    keep_alive = 1;
    remain     = 0;
    parse_end  = 0;
    rdlen      = 0;
    rbuf_size  = sizeof(rbuf);
    wbuf_size  = sizeof(wbuf);
    l_og(e_log_level_debug, "reading fd: %d\n", conn->socket);
    if(conn->data_len > 0)
    {
        memcpy(rbuf, conn->data_buffer, conn->data_len);
        remain = conn->data_len;
    }
    do
    {
        ret = recv(conn->socket, rbuf + remain, rbuf_size - remain, MSG_DONTWAIT);
        if(ret < 0)
        {
            l_og(e_log_level_debug, "socket: %d return %d\n", conn->socket, ret);
            if(errno != EWOULDBLOCK && errno != EAGAIN)
            {
                l_og_error("recv");
                keep_alive = 0;
            }
            else if(remain)
            {
                if(remain > 1024)
                    l_og(e_log_level_fatal, "IMPOSSIBLE!!! remain > 1024\n");
                conn->data_len = remain > 1024 ? 1024 : remain;
                memcpy(conn->data_buffer, rbuf, conn->data_len);
            }
            break;
        }
        if(ret == 0)
        {
            l_og(e_log_level_info, "connection closing... address: %s, port: %d, socket: %d\n",
                    inet_ntoa(((struct sockaddr_in*)&conn->peer_addr)->sin_addr), 
                    ntohs(((struct sockaddr_in*)&conn->peer_addr)->sin_port), conn->socket);
            keep_alive = 0;
            break;
        }
        l_og(e_log_level_debug, "socket %d received size: %d\n", conn->socket, ret);
        if(http_request_parse(&conn->request, rbuf, ret + remain, &rdlen, &parse_end) < 0)
        {
            l_og(e_log_level_debug, "http_request_parse failed\n");
        }
        else
        {
            remain += ret - rdlen;
            l_og(e_log_level_debug, "rdlen: %d\n", rdlen);
            l_og(e_log_level_debug, "remain: %d\n", remain);
            if(rdlen && remain)
            {
                if(rdlen >= remain)
                {
                    memcpy(rbuf, rbuf + rdlen, remain);
                }
                else
                {
                    for(wp = 0; wp < remain; ++wp)
                    {
                        rbuf[wp] = rbuf[wp + rdlen];
                    }
                }
            }
        }
        if(parse_end)
        {
            http_request_log(&conn->request);
            server_process_request(conn->srv, &conn->request, &conn->response, &keep_alive);
            wp = 0;
            send_end = 0;
            http_response_log(&conn->response);
            do
            {
                ret = http_response_gen(&conn->response, wbuf, wbuf_size, &send_end);
                if(ret <= 0)
                {
                    l_og(e_log_level_debug, "http_response_gen return %d\n", ret);
                }
                else
                {
                    ret = write(conn->socket, wbuf, ret);
                    if(ret < 0)
                    {
                        keep_alive = 0;
                        break;
                    }
                    wp += ret;
                }
            }while(!send_end);
            l_og(e_log_level_info, "File: %-30s transfered, %-8d bytes\n", conn->request.url.url, wp);
            http_request_reset(&conn->request);
            http_response_reset(&conn->response, e_http_200_ok);
        }
    }while(keep_alive);
    if(!keep_alive)
    {
        l_og(e_log_level_debug, "server_serv_client end keep_alive: %d socket: %d\n", keep_alive, conn->socket);
        close(conn->socket);
        conn->socket = -1;
    }
    else
    {
        pthread_mutex_lock(&conn->srv->fd_lock);
        l_og(e_log_level_debug, "socket %d is return\n", conn->socket);
        FD_SET(conn->socket, &conn->srv->fds);
        pthread_mutex_unlock(&conn->srv->fd_lock);
        server_notify(conn->srv);
    }
}

void server_process_request(server_t *srv, http_request_t *req, http_response_t *res, int *keep_alive)
{
    if(req->parse_status == e_http_request_parse_status_error)
    {
        *keep_alive = 0;
        http_response_reset(res, e_http_400_bad_request);
        http_header_add_by_id(&res->headers, e_header_content_length, "0", 1);
    }
    else
    {
        switch(req->method)
        {
            case e_http_get:
                server_process_get(srv, req, res, keep_alive);
                break;
            case e_http_head:
                server_process_head(srv, req, res, keep_alive);
                break;
            case e_http_options:
                server_process_options(srv, req, res, keep_alive);
                break;
            case e_http_put:
            case e_http_post:
            default:
                http_response_reset(res, e_http_405_method_not_allowed);
                http_header_add_by_id(&res->headers, e_header_content_length, "0", 1);
                break;
        }
    }
    http_header_add_by_id(&res->headers, e_header_connection, *keep_alive ? "keep-alive" : "close", *keep_alive ? 10: 5);
    http_header_add_by_id(&res->headers, e_header_server, "O's Server", 10);
}
void gen_path(http_url_path_seg_t *node, char **path)
{
    memcpy(*path, node->data, node->data_len);
    *path = *path + node->data_len;
    **path = 0;
}
void server_process_get(server_t *srv, http_request_t *req, http_response_t *res, int *keep_alive)
{
    char path[PATH_MAX + 1], tmp[PATH_MAX + 1], *type, *path_p;
    int clen, len, fd;
    struct stat file_info;

    clen = 0;
    http_response_reset(res, e_http_200_ok);
    strcpy(path, srv->config.root);
    path_p = path + strlen(srv->config.root);
    *path_p++ = '/';
    path_p[0] = 0;
    list_traverse((list_t*)&req->url.path.path_stack, (list_visit_func)gen_path, &path_p);
    if(*(path_p - 1) == '/')
    {
        strcpy(path_p, "index.html");
    }
    type = strrchr(path, '.');
    if(NULL != type)
    {
        ++type;
    }
    l_og(e_log_level_debug, "file path: %s\n", path);
    if(0 != stat(path, &file_info))
    {
        l_og_error("stat");
        res->status = e_http_404_not_found;
    }
    else if(!S_ISREG(file_info.st_mode))
    {
        l_og(e_log_level_debug, "%s not a file\n", path);
        res->status = e_http_404_not_found;
    }
    else if(!file_info.st_size)
    {
        l_og(e_log_level_debug, "%s empty\n", path);
        res->status = e_http_200_ok;
    }
    else if((fd = open(path, O_RDONLY)) < 0)
    {
        l_og_error("open");
        res->status = e_http_500_internal_server_error;
    }
    else
    {
        res->content = fd;
        clen = file_info.st_size;
        *keep_alive = 1;
    }
    len = sprintf(tmp, "%d", clen);
    http_header_add_by_id(&res->headers, e_header_content_length, tmp, len);
    if(NULL != type)
    {
        if(strcmp(type, "jpg") == 0 || strcmp(type, "jpeg") == 0
                || strcmp(type, "png") == 0 || strcmp(type, "bmp") == 0)
            len = sprintf(tmp, "image/%s", type);
        else
            len = sprintf(tmp, "text/%s", type);
        http_header_add_by_id(&res->headers, e_header_content_type, tmp, len);
    }
}
void server_process_head(server_t *srv, http_request_t *req, http_response_t *res, int *keep_alive)
{
    server_process_get(srv, req, res, keep_alive);
    if(res->content >= 0)
    {
        close(res->content);
        res->content = -1;
    }
}
void server_process_options(server_t *srv, http_request_t *req, http_response_t *res, int *keep_alive)
{
    http_response_reset(res, e_http_200_ok);
    http_header_add_by_id(&res->headers, e_header_allow, "OPTIONS, GET, PUT, HEAD", 23);
    http_header_add_by_id(&res->headers, e_header_content_length, "0", 1);
}
