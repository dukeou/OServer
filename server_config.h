#ifndef O_SERVER_CONFIG
#define O_SERVER_CONFIG
#include <limits.h>
#define O_SERVER_DEFAULT_PORT 8080
typedef struct
{
    int num_regular_threads;
    int num_max_threads;
    int num_max_connections;
    int port;
    int backlog;
    int stale_connection_timeout_sec;
    char root[PATH_MAX + 1];
}server_config_t;
int server_config_read(server_config_t *cfg, int argc, char * const argv[]);
void server_config_help(char *program);
#endif /* O_SERVER_CONFIG */
